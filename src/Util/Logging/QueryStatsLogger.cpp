#include "Util/typedefs.hpp"
#include "Util/Logging/QueryStatsLogger.hpp"

using QSTC = QueryStatsColumn;

void QueryStats::calculate() {
    m_dist_stats.calculate(U(m_subs_count));
    m_rc_using_max = (m_dist_stats.m_max - m_dist_stats.m_min) / m_dist_stats.m_min;
    m_rc_using_mean = m_dist_stats.m_mean / m_dist_stats.m_min;
}

void QueryStatsLogger::write_entry(uint query_id, const vec<vec<Real>> &query, QueryStats stats, bool normalized) {
#ifndef DISABLE_LOGGING
    QueryStatsLogger instance;
    auto &RS = RunSettings::get_instance();

    str dataset_file = RS.m_dataset_props.m_file;
    str query_file = RS.m_query_properties.m_file;

    size_t query_len = 0;
    str query_channels_str = "";
    for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
        query_channels_str += query[c].empty() ? "0" : "1";
        if (c < query.size() - 1) query_channels_str += instance.ITEM_SEP;
        query_len = std::max(query_len, query[c].size());
    }
    str query_len_str = to_string(query_len);

    str query_stats_path = fs::path(RS.get_logs_path()) / instance.QUERY_STATS_FILE;
    instance.file_setup(query_stats_path, QUERY_STATS_COL_STRS);
    instance.write_row(query_stats_path,
                       {
                           {QSTC::ID, to_string(query_id)},
                           {QSTC::DATASET_FILE, dataset_file},
                           {QSTC::QUERY_FILE, query_file},
                           {QSTC::QUERY_LENGTH, query_len_str},
                           {QSTC::QUERY_CHANNELS, query_channels_str},
                           ADD_STATS_TO_ROW(QSTC, DIST, stats.m_dist_stats),
                           {QSTC::RC_USING_MAX, to_string(stats.m_rc_using_max)},
                           {QSTC::RC_USING_MEAN, to_string(stats.m_rc_using_mean)},
                           {QSTC::NORMALIZED, to_string(normalized)},
                       },
                       QUERY_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}
