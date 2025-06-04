#include "Util/Logging/IndexLogger.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

IndexLogger IndexLogger::instance = IndexLogger();
bool IndexLogger::initialized = false;

using ISC = IndexSettingsColumn;

const str IndexLogger::INDEX_SETTINGS_FILE = "index_settings.csv";

void IndexLogger::initialize(const IndexOptions &index_options, Real sample_frac) {
    if (initialized) return;

    initialized = true;

    auto &RS = RunSettings::get_instance();
    instance.m_index_settings_path =
        fs::path(RunSettings::get_instance().get_logs_path()) / instance.INDEX_SETTINGS_FILE;
    instance.file_setup(instance.m_index_settings_path, INDEX_SETTINGS_COL_STRS);

    uint num_segments = 0, pos_per_env = 0, sampling_chss_sample_size = 0, sampling_chss_segment_len = 0;
    SaxNumBitsT first_layer_num_bits = 0, num_bits_limit = 0, merger_num_bits = 0;
    size_t leaf_capacity = 0;
    str lg_ss_str = "", ch_ss_str = "", ss_str = "", brs_str = "", sps_str = "", min_num_bits_on_tie_str = "",
        merge_in_leaves_str = "", method_type_str = "", entry_merger_type_str = "", env_stats_chss_weights_file = "",
        multi_chss_num_seg_file = "", env_width_chss_min_w_update_str = "", max_width_change_str = "";
    Real score_based_chss_score_exp = 0.0;

    Real index_size_limit = index_options.m_index_method == ENVELOPE ? index_options.m_index_size_limit : 0.0;

    if (index_options.m_index_params && arr_contains(METHODS_W_PAA, index_options.m_index_method)) {
        auto method_type = index_options.m_index_params->get_type();
        method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(method_type);

        auto *paa_params = dynamic_cast<PaaIndexParams *>(index_options.m_index_params.get());
        lg_ss_str = LENGTH_GROUP_SEGMENTATION_STRATEGY_TO_STR.at(paa_params->m_segmentation_params.m_lg_strategy_type);
        ch_ss_str = CHANNEL_SEGMENTATION_STRATEGY_TO_STR.at(paa_params->m_segmentation_params.m_ch_strategy_type);

        if (auto score_based_chss_params = paa_params->m_segmentation_params.m_score_based_chss_params) {
            sampling_chss_sample_size = score_based_chss_params->m_sample_size;
            sampling_chss_segment_len = score_based_chss_params->m_segment_len;
            score_based_chss_score_exp = score_based_chss_params->m_score_exp;
            if (score_based_chss_params->m_env_scores_type == STATS)
                env_stats_chss_weights_file = score_based_chss_params->m_weights_file;
            if (score_based_chss_params->m_env_scores_type == WIDTH)
                env_width_chss_min_w_update_str = to_string(score_based_chss_params->m_min_width_update);
        }

        multi_chss_num_seg_file = paa_params->m_segmentation_params.m_ch_num_seg_props_file;
        ss_str = SEGMENTATION_STRATEGY_TO_STR.at(paa_params->m_segmentation_params.m_strategy_type);
        num_segments = paa_params->m_segmentation_params.m_num_segments;

        auto entry_merger_type = paa_params->m_merger_params.m_entry_merger_type;
        entry_merger_type_str = ENTRY_MERGER_TYPE_TO_STR.at(entry_merger_type);
        if (arr_contains(MERGERS_W_SAX, entry_merger_type)) {
            merger_num_bits = paa_params->m_merger_params.m_merger_sax_params->m_num_bits;
        }

        if (arr_contains(METHODS_W_SAX, method_type)) {
            auto *sax_index_params = dynamic_cast<SaxIndexParams *>(index_options.m_index_params.get());
            first_layer_num_bits = sax_index_params->m_sax_params.m_num_bits;
            brs_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(sax_index_params->m_sax_params.m_breakpoint_strategy_type);

            if (arr_contains(METHODS_W_ISAX, method_type)) {
                auto *isax_index_params = dynamic_cast<iSaxIndexParams *>(index_options.m_index_params.get());
                leaf_capacity = isax_index_params->m_isax_trie_params.m_leaf_capacity;

                auto split_strategy = isax_index_params->m_isax_trie_params.m_split_strategy_type;
                sps_str = ISAX_SPLIT_STRATEGY_TO_STR.at(split_strategy);

                merge_in_leaves_str = to_string(isax_index_params->m_isax_trie_params.m_merge_in_leaves);
                if (split_strategy == ENTROPY_MAXIMIZING)
                    min_num_bits_on_tie_str = to_string(isax_index_params->m_isax_trie_params.m_min_num_bits_on_tie);
                num_bits_limit = isax_index_params->m_isax_trie_params.m_num_bits_limit;
            }

            if (arr_contains(METHODS_W_ENV_GROUPING, method_type)) {
                auto grouping_params =
                    dynamic_cast<TreeEnvelopeIndexParams *>(index_options.m_index_params.get())->m_env_grouping_params;
                if (method_type == TREE_ENVELOPE || method_type == BUCKETING_ENVELOPE)
                    leaf_capacity = grouping_params.m_bucket_size;
                else if (method_type == VL_ENVELOPE)
                    max_width_change_str = to_string(grouping_params.m_max_width_change);
            }
        }
        if (arr_contains(METHODS_W_ENVELOPE, method_type)) {
            auto *env_params = dynamic_cast<EnvelopeIndexParams *>(index_options.m_index_params.get());
            pos_per_env = env_params->m_pos_per_env;
        }
    }

    instance.m_columns = {
        {ISC::ID, to_string(instance.determine_index(instance.m_index_settings_path))},
        {ISC::DATASET_FILE, RS.m_dataset_props.m_file},
        {ISC::INDEX_FILE, RS.m_index_file},
        {ISC::FFTS_FILE, RS.m_ffts_file},
        {ISC::L_MIN, format_num_param(index_options.m_l_min)},
        {ISC::L_MAX, format_num_param(index_options.m_l_max)},
        {ISC::L_PER_GROUP, format_num_param(index_options.m_l_per_group)},
        {ISC::NORMALIZED, to_string(index_options.m_normalized)},
        {ISC::INDEX_TYPE, method_type_str},
        {ISC::NUM_SEGMENTS, to_string(num_segments)},
        {ISC::MULTI_CHSS_NUM_SEG_FILE, multi_chss_num_seg_file},
        {ISC::SCORE_BASED_CHSS_SCORE_EXP, format_num_param(score_based_chss_score_exp)},
        {ISC::SCORE_BASED_CHSS_SEGMENT_LEN, format_num_param(sampling_chss_segment_len)},
        {ISC::SCORE_BASED_CHSS_SAMPLE_SIZE, format_num_param(sampling_chss_sample_size)},
        {ISC::ENV_STATS_SCORE_WEIGHTS_FILE, env_stats_chss_weights_file},
        {ISC::ENV_WIDTH_SCORE_MIN_W_UPDATE, env_width_chss_min_w_update_str},
        {ISC::POS_PER_ENV, format_num_param(pos_per_env)},
        {ISC::INDEX_SIZE_LIMIT, format_num_param(index_size_limit)},
        {ISC::ENTRY_MERGER_TYPE, entry_merger_type_str},
        {ISC::MERGER_NUM_BITS, format_num_param(merger_num_bits)},
        {ISC::FIRST_LAYER_NUM_BITS, format_num_param(first_layer_num_bits)},
        {ISC::LEAF_CAPACITY, format_num_param(leaf_capacity)},
        {ISC::MAX_WIDTH_CHANGE, max_width_change_str},
        {ISC::LG_SEGMENTATION_STRATEGY, lg_ss_str},
        {ISC::CH_SEGMENTATION_STRATEGY, ch_ss_str},
        {ISC::SEGMENTATION_STRATEGY, ss_str},
        {ISC::BREAKPOINT_STRATEGY, brs_str},
        {ISC::SPLIT_STRATEGY, sps_str},
        {ISC::MERGE_IN_LEAVES, merge_in_leaves_str},
        {ISC::MIN_NUM_BITS_ON_TIE, min_num_bits_on_tie_str},
        {ISC::NUM_BITS_LIMIT, format_num_param(num_bits_limit)},
        {ISC::ADAPT_TO_DATASET, to_string(index_options.m_adapt)},
        {ISC::INSERTER_TYPE, ENTRY_INSERTER_TYPE_TO_STR.at(index_options.m_inserter_type)},
        {ISC::SAMPLE_FRAC, sample_frac < 1.0 ? to_string(sample_frac) : ""},
    };
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_count_cols[col] = 0;
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_time_cols_duration[col] = 0;
}

void IndexLogger::set_pos_per_env(uint pos_per_env) { m_columns[ISC::POS_PER_ENV] = format_num_param(pos_per_env); }

void IndexLogger::set_num_segments_cols(const ILengthGroupSegmentationStrategy *lg_segmentation_strategy,
                                        bool log_num_seg_per_ch, bool log_num_seg_all) {
    auto &RS = RunSettings::get_instance();

    MtsNumChannelsT num_channels = RS.m_dataset_props.m_num_channels;
    uint num_len_groups = RS.m_length_props.m_num_l_groups;

    vec<SaxSegIndT> num_seg_per_ch, num_seg_all;
    if (log_num_seg_per_ch) {
        num_seg_per_ch.reserve(num_channels);
        uint l_max = RS.m_length_props.m_l_max;
        uint lg_ind = RS.m_length_props.m_use_length_groups ? num_len_groups - 1 : 0;
        auto chss = lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind);
        for (MtsNumChannelsT ch_ind = 0; ch_ind < num_channels; ++ch_ind) {
            num_seg_per_ch.push_back(chss->get_const_segmentation_strategy(ch_ind)->get_num_segments(l_max));
        }
    }
    if (log_num_seg_all) {
        num_seg_all.reserve(num_len_groups * num_channels);
        for (uint lg_ind = 0; lg_ind < num_len_groups; ++lg_ind) {
            auto chss = lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind);
            uint lg_l_max = RS.get_lg_l_max(lg_ind);
            for (MtsNumChannelsT ch_ind = 0; ch_ind < num_channels; ++ch_ind) {
                num_seg_all.push_back(chss->get_const_segmentation_strategy(ch_ind)->get_num_segments(lg_l_max));
            }
        }
    }

    m_columns[ISC::NUM_SEGMENTS_PER_CHANNEL] = get_collection_str(num_seg_per_ch);
    m_columns[ISC::NUM_SEGMENTS_ALL] = get_collection_str(num_seg_all);
}

void IndexLogger::write_entry() {
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_columns[col] = to_string(instance.m_count_cols[col]);
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_columns[col] = to_string(instance.m_time_cols_duration[col]);
    instance.write_row(m_index_settings_path, instance.m_columns, INDEX_SETTINGS_COL_ENUMS);
}
