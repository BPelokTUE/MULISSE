#include "Util/Logging/IndexLogger.hpp"

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

    uint num_segments = 0, pos_per_env = 0;
    SaxNumBitsT first_layer_num_bits = 0, num_bits_limit = 0, merger_num_bits = 0;
    size_t leaf_capacity = 0;
    str lg_ss_str = "", ch_ss_str = "", ss_str = "", brs_str = "", sps_str = "", min_num_bits_on_tie_str = "",
        merge_in_leaves_str = "", method_type_str = "", entry_merger_type_str = "", ch_score_based_weights_file = "",
        ch_num_seg_props_file = "";
    Real ch_score_based_prop_exp = 0.0;

    if (index_options.m_index_params && arr_contains(METHODS_W_PAA, index_options.m_index_method)) {
        auto method_type = index_options.m_index_params->get_type();
        method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(method_type);

        auto *paa_params = dynamic_cast<PaaIndexParams *>(index_options.m_index_params.get());
        lg_ss_str = LENGTH_GROUP_SEGMENTATION_STRATEGY_TO_STR.at(paa_params->m_segmentation_params.m_lg_strategy_type);
        ch_ss_str = CHANNEL_SEGMENTATION_STRATEGY_TO_STR.at(paa_params->m_segmentation_params.m_ch_strategy_type);
        if (auto ch_env_stats_params =
                dynamic_cast<const EnvStatsChSSParams *>(paa_params->m_segmentation_params.m_ch_score_based_params)) {
            ch_score_based_weights_file = ch_env_stats_params->m_weights_file;
            ch_score_based_prop_exp = ch_env_stats_params->m_prop_exp;
        }
        ch_num_seg_props_file = paa_params->m_segmentation_params.m_ch_num_seg_props_file;
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
            } else if (method_type == TREE_ENVELOPE) {
                auto *tree_env_params = dynamic_cast<TreeEnvelopeIndexParams *>(index_options.m_index_params.get());
                leaf_capacity = tree_env_params->m_bucket_size;
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
        {ISC::CH_SCORE_BASED_WEIGHTS_FILE, ch_score_based_weights_file},
        {ISC::CH_SCORE_BASED_PROP_EXP, format_num_param(ch_score_based_prop_exp)},
        {ISC::CH_NUM_SEG_PROPS_FILE, ch_num_seg_props_file},
        {ISC::POS_PER_ENV, format_num_param(pos_per_env)},
        {ISC::ENTRY_MERGER_TYPE, entry_merger_type_str},
        {ISC::MERGER_NUM_BITS, format_num_param(merger_num_bits)},
        {ISC::FIRST_LAYER_NUM_BITS, format_num_param(first_layer_num_bits)},
        {ISC::LEAF_CAPACITY, format_num_param(leaf_capacity)},
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

void IndexLogger::write_entry() {
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_columns[col] = to_string(instance.m_count_cols[col]);
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_columns[col] = to_string(instance.m_time_cols_duration[col]);
    instance.write_row(m_index_settings_path, instance.m_columns, INDEX_SETTINGS_COL_ENUMS);
}
