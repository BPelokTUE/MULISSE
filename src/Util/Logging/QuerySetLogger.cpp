#include "Util/Logging/QuerySetLogger.hpp"

#include "Search/QueryGenOptions.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Artefacts/Properties/MtsQuerySetProperties.hpp"

using QSC = QuerySetSettingsColumn;

const str QuerySetLogger::QUERY_SET_SETTINGS_FILE = "query_set_settings.csv";

void QuerySetLogger::write_entry(const MtsDatasetProperties &dataset_settings,
                                 const MtsQuerySetProperties &query_set_settings,
                                 const QuerySetGenOptions &query_gen_opts, const str logs_path) {
#ifndef DISABLE_LOGGING
    QuerySetLogger instance;

    str query_settings_path = fs::path(logs_path) / QuerySetLogger::QUERY_SET_SETTINGS_FILE;
    instance.file_setup(query_settings_path, QUERY_SET_SETTINGS_COL_STRS);

    instance.write_row(query_settings_path,
                       {
                           {QSC::ID, to_string(instance.determine_index(query_settings_path))},
                           {QSC::DATASET_FILE, dataset_settings.m_dataset_path},
                           {QSC::QUERY_FILE, RS.m_query_file},
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
