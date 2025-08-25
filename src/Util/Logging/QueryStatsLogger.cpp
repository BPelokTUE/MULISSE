#include "Util/Logging/QueryStatsLogger.hpp"

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Stats/QueryStats.hpp"
#include "Util/Types/MtsQuery.hpp"

using QSTC = QueryStatsColumn;

QueryStatsLogger::QueryStatsLogger(const str &logs_path) {
    m_query_stats_path = fs::path(logs_path) / QUERY_STATS_FILE;
    m_base_id = determine_index(m_query_stats_path);
    file_setup(m_query_stats_path, QUERY_STATS_COL_STRS);
}

uint QueryStatsLogger::write_entry(const MtsQuerySet &query_set, const MtsQuery &query, const QueryStats &query_stats) {
#ifndef DISABLE_LOGGING
    str query_channels_str = "";
    MtsNumChannelsT num_channels = query.get_num_channels();
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        query_channels_str += query.is_channel_used(c) ? "1" : "0";
        if (c < num_channels - 1) query_channels_str += ITEM_SEP;
    }

    write_row(m_query_stats_path,
              {
                  {QSTC::ID, to_string(m_base_id)},
                  {QSTC::QUERY_SET_ID, to_string(query_set.get_log_id())},
                  {QSTC::QUERY_FILE, query_set.get_properties().m_query_set_path},
                  {QSTC::QUERY_LENGTH, to_string(query.get_query_len())},
                  {QSTC::QUERY_CHANNELS, query_channels_str},
                  ADD_STATS_TO_ROW(QSTC, DIST, query_stats.m_dist_stats),
                  {QSTC::RC_USING_MAX, to_string(query_stats.m_rc_using_max)},
                  {QSTC::RC_USING_MEAN, to_string(query_stats.m_rc_using_mean)},
                  {QSTC::NORMALIZED, to_string(query.is_normalized())},
              },
              QUERY_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
    return m_base_id++;
}
