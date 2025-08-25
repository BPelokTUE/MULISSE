#include "Util/Logging/QuerySetLogger.hpp"

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/Artefacts/Options/QuerySetGenOptions.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Artefacts/Properties/MtsQuerySetProperties.hpp"

using QSC = QuerySetSettingsColumn;

QuerySetLogger::QuerySetLogger(const str &logs_path) {
    m_query_set_settings_path = fs::path(logs_path) / QUERY_SET_SETTINGS_FILE;
    file_setup(m_query_set_settings_path, QUERY_SET_SETTINGS_COL_STRS);
}

uint QuerySetLogger::write_entry(const MtsQuerySet &query_set, const QuerySetGenOptions &query_gen_opts) {
    uint id = determine_index(m_query_set_settings_path);
#ifndef DISABLE_LOGGING
    auto &query_set_props = query_set.get_properties();
    auto &dataset = query_set.get_source_dataset();

    write_row(m_query_set_settings_path,
              {
                  {QSC::ID, to_string(id)},
                  {QSC::DATASET_ID, to_string(dataset.get_log_id())},
                  {QSC::DATASET_FILE, dataset.get_properties().m_dataset_path},
                  {QSC::QUERY_FILE, query_set_props.m_query_set_path},
                  {QSC::NUM_QUERIES, to_string(query_set_props.m_num_queries)},
                  {QSC::L_MIN, format_num_param(query_set_props.m_length_range.m_l_min)},
                  {QSC::L_MAX, format_num_param(query_set_props.m_length_range.m_l_max)},
                  {QSC::EXACT_LENGTHS, get_collection_str(query_gen_opts.m_exact_lengths)},
                  {QSC::USED_CHANNELS, format_num_param(query_gen_opts.m_used_channels)},
                  {QSC::CHANNEL_MASK, get_collection_str(query_gen_opts.m_channel_mask)},
                  {QSC::NOISE, to_string(query_gen_opts.m_noise)},
                  {QSC::SEED, to_string(query_gen_opts.m_seed)},
              },
              QUERY_SET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
    return id;
}
