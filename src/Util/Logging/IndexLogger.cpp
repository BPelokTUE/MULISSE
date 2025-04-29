#include "Util/typedefs.hpp"
#include "Util/Logging/IndexLogger.hpp"

IndexLogger IndexLogger::instance = IndexLogger();
bool IndexLogger::initialized = false;

using ISC = IndexSettingsColumn;

void IndexLogger::initialize(const IndexOptions &index_options) {
    if (initialized) return;
    initialized = true;

    auto &RS = RunSettings::get_instance();
    instance.m_index_settings_path =
        fs::path(RunSettings::get_instance().get_logs_path()) / instance.INDEX_SETTINGS_FILE;
    instance.file_setup(instance.m_index_settings_path, INDEX_SETTINGS_COL_STRS);

    uint num_segments = 0, pos_per_env = 0;
    SaxNumBitsT first_layer_num_bits = 0, num_bits_limit = 0;
    size_t leaf_capacity = 0;
    str ss_str = "", brs_str = "", sps_str = "", min_num_bits_on_tie_str = "", method_type_str = "";

    if (index_options.m_index_params) {
        auto method_type = index_options.m_index_params->get_type();
        method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(method_type);

        auto *paa_params = dynamic_cast<PaaIndexParams *>(index_options.m_index_params.get());
        num_segments = paa_params->m_num_segments;
        ss_str = SEGMENTATION_STRATEGY_TO_STR.at(paa_params->m_segmentation_strategy_type);

        if (std::find(METHODS_W_SAX.begin(), METHODS_W_SAX.end(), method_type) != METHODS_W_SAX.end()) {
            auto *sax_params = dynamic_cast<SaxIndexParams *>(index_options.m_index_params.get());
            first_layer_num_bits = sax_params->m_num_bits;
            brs_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(sax_params->m_breakpoint_strategy_type);

            if (std::find(METHODS_W_ISAX.begin(), METHODS_W_ISAX.end(), method_type) != METHODS_W_ISAX.end()) {
                auto *isax_params = dynamic_cast<iSaxIndexParams *>(index_options.m_index_params.get());
                leaf_capacity = isax_params->m_leaf_capacity;

                auto split_strategy = isax_params->m_split_strategy_type;
                sps_str = ISAX_SPLIT_STRATEGY_TO_STR.at(split_strategy);

                if (split_strategy == ENTROPY_MAXIMIZING)
                    min_num_bits_on_tie_str = to_string(isax_params->m_min_num_bits_on_tie);
                num_bits_limit = isax_params->m_num_bits_limit;
            } else if (method_type == TREE_ENVELOPE) {
                auto *tree_env_params = dynamic_cast<TreeEnvelopeIndexParams *>(index_options.m_index_params.get());
                leaf_capacity = tree_env_params->m_bucket_size;
            }
        }
        if (std::find(METHODS_W_ENVELOPE.begin(), METHODS_W_ENVELOPE.end(), method_type) != METHODS_W_ENVELOPE.end()) {
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
        {ISC::POS_PER_ENV, format_num_param(pos_per_env)},
        {ISC::FIRST_LAYER_NUM_BITS, format_num_param(first_layer_num_bits)},
        {ISC::LEAF_CAPACITY, format_num_param(leaf_capacity)},
        {ISC::SEGMENTATION_STRATEGY, ss_str},
        {ISC::BREAKPOINT_STRATEGY, brs_str},
        {ISC::SPLIT_STRATEGY, sps_str},
        {ISC::MIN_NUM_BITS_ON_TIE, min_num_bits_on_tie_str},
        {ISC::NUM_BITS_LIMIT, format_num_param(num_bits_limit)},
        {ISC::ADAPT_TO_DATASET, to_string(index_options.m_adapt)},
        {ISC::INSERTER_TYPE, ENTRY_INSERTER_TYPE_TO_STR.at(index_options.m_inserter_type)},
    };
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_count_cols[col] = 0;
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_time_cols_duration[col] = 0;
}

void IndexLogger::write_entry() {
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_columns[col] = to_string(instance.m_count_cols[col]);
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_columns[col] = to_string(instance.m_time_cols_duration[col]);
    instance.write_row(m_index_settings_path, instance.m_columns, INDEX_SETTINGS_COL_ENUMS);
}
