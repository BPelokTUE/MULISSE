#ifndef QUERY_STATSS_LOGGER_HPP
#define QUERY_STATSS_LOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Stats/AttributesStats.hpp"

class MtsDataset;
class MtsQuerySet;
class MtsQuery;
struct QueryStats;

/** @brief Enum of the columns of the query statistics log file */
enum class QueryStatsColumn {
    ID,              // ID of the query within the query file
    DATASET_ID,      // ID of the dataset the query was generated from
    DATASET_FILE,    // Name of the dataset file the query were generated from - required for backward compatibility
    QUERY_SET_ID,    // ID of the query set the query was generated from
    QUERY_FILE,      // Name of the query file
    QUERY_LENGTH,    // Length of the query
    QUERY_CHANNELS,  // Channels included in the query as a list of ITEM_SEP separated `0`s and `1`s
    NORMALIZED,      // Whether the query and subsequences are normalized
    DEFINE_STAT_COLUMNS(DIST),  // Statistics of the distances of the query to subsequences in the dataset
    RC_USING_MAX,               // Relative contrast of the query, calculated as (D_max - D_min) / D_min
    RC_USING_MEAN,              // Relative contrast of the query, calculated as D_mean / D_min
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QueryStatsColumn, QUERY_STATS_COL, false);

// QueryStatsLogger class

/** @brief Class for logging query statistics */
class QueryStatsLogger : public Logger {
   public:
    /**
     * @brief Constructor
     * @param logs_path The path to the logs directory
     */
    QueryStatsLogger(const str &logs_path);

    /**
     * @brief Write a query statistics entry
     * @param query_set The query set the query belongs to
     * @param query The query to log statistics for
     * @param query_stats The statistics of the query
     * @return The ID of the entry within the log file, or 0 if logging is disabled
     */
    uint write_entry(const MtsQuerySet &query_set, const MtsQuery &query, const QueryStats &query_stats);

   private:
    static const str QUERY_STATS_FILE;

    str m_query_stats_path;

    uint m_base_id;
};

#endif  // QUERY_STATSS_LOGGER_HPP
