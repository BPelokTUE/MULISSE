#include "Util/typedefs.hpp"
#include "Util/Logging/QuerySetLogger.hpp"

using QSC = QuerySetSettingsColumn;

void QuerySetLogger::write_entry(QuerySetOptions &opts) {
#ifndef DISABLE_LOGGING
    QuerySetLogger instance;

    auto &RS = RunSettings::get_instance();

    str query_settings_path = fs::path(RS.get_logs_path()) / instance.QUERY_SET_SETTINGS_FILE;
    instance.file_setup(query_settings_path, QUERY_SET_SETTINGS_COL_STRS);

    instance.write_row(query_settings_path,
                       {
                           {QSC::ID, to_string(instance.determine_index(query_settings_path))},
                           {QSC::DATASET_FILE, RS.get_dataset_props().m_file},
                           {QSC::QUERY_FILE, RS.get_query_path()},
                           {QSC::NUM_QUERIES, to_string(opts.m_num_queries)},
                           {QSC::L_MIN, format_num_param(opts.m_l_min)},
                           {QSC::L_MAX, format_num_param(opts.m_l_max)},
                           {QSC::EXACT_LENGTHS, instance.get_collection_str(opts.m_exact_lengths)},
                           {QSC::USED_CHANNELS, format_num_param(opts.m_used_channels)},
                           {QSC::CHANNEL_MASK, instance.get_collection_str(opts.m_channel_mask)},
                           {QSC::NOISE, to_string(opts.m_noise)},
                           {QSC::SEED, to_string(opts.m_seed)},
                       },
                       QUERY_SET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}