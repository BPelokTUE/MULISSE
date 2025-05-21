#ifndef QUERY_STATSS_LOGGER_HPP
#define QUERY_STATSS_LOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Stats/AttributesStats.hpp"

/** @brief Enum of the columns of the query statistics log file */
enum class QueryStatsColumn {
    ID,              // ID of the query within the query file
    DATASET_FILE,    // Name of the dataset file the query were generated from - required for backward compatibility
    QUERY_FILE,      // Name of the query file
    QUERY_LENGTH,    // Length of the query
    QUERY_CHANNELS,  // Channels included in the query as a list of ITEM_SEP separated `0`s and `1`s
    NORMALIZED,      // Whether the query and subsequences are normalized
    DEFINE_STAT_COLUMNS(DIST),  // Statistics of the distances of the query to subsequences in the dataset
    RC_USING_MAX,               // Relative contrast of the query, calculated as (D_max - D_min) / D_min
    RC_USING_MEAN,              // Relative contrast of the query, calculated as D_mean / D_min
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QueryStatsColumn, QUERY_STATS_COL, false);

// QueryStats class

struct QueryStats {
    AttributeStats m_dist_stats;
    size_t m_subs_count = 0;
    Real m_rc_using_max, m_rc_using_mean;

    QueryStats() = default;

    void calculate();
};

// QueryStatsLogger class

/** @brief Class for logging query statistics */
class QueryStatsLogger : public Logger {
   public:
    /**
     * @brief Write a query statistics entry
     * @param query_id The ID of the query
     * @param query The query
     * @param query_stats The statistics of the query
     * @param normalized Whether the query and subsequences are normalized
     * */
    static void write_entry(uint query_id, const vec<vec<Real>> &query, QueryStats stats, bool normalized);

   private:
    QueryStatsLogger() = default;

    static const str QUERY_STATS_FILE;
};

#endif  // QUERY_STATSS_LOGGER_HPP
