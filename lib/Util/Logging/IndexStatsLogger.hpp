#ifndef INDEX_STATS_LOGGER_HPP
#define INDEX_STATS_LOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Stats/IndexStats.hpp"

/** @brief Enum of the columns of the index statistics log file */
enum class IndexStatsColumn {
    INDEX_FILE,                           // Name of the index file / directory
    LENGTH_GROUP_ID,                      // ID of the length group within the index (0 if no length grouping is used)
    SUB_INDEX_ID,                         // ID of the sub-index within the index
    DEFINE_STAT_COLUMNS(LEAF_SIZE),       // Statistics of the sizes of the leaves / # entries in the leaves
    DEFINE_STAT_COLUMNS(LEAF_HEIGHT),     // Statistics of the height of the leaves
    DEFINE_STAT_COLUMNS(SEG_RANGE),       // Statistics of the range of the segments
    DEFINE_STAT_COLUMNS(SEG_LOWER),       // Statistics of the lower bound of the segments
    DEFINE_STAT_COLUMNS(SEG_UPPER),       // Statistics of the upper bound of the segments
    DEFINE_STAT_COLUMNS(SEG_MID),         // Statistics of the (lower+upper)/2 of the segments
    DEFINE_STAT_COLUMNS(SEG_RANGE_LIST),  // Statistics of the range of the segments, per segment position
    DEFINE_STAT_COLUMNS(SEG_LOWER_LIST),  // Statistics of the lower bound of the segments, per segment position
    DEFINE_STAT_COLUMNS(SEG_UPPER_LIST),  // Statistics of the upper bound of the segments, per segment position
    DEFINE_STAT_COLUMNS(SEG_MID_LIST),    // Statistics of the (lower+upper)/2 of the segments, per segment position
    SEGMENT_COUNT_LIST,                   // Number of entries per segment x channel
    NUM_INF_LOWER,                        // Number of segments with `-INF` as the lower bound
    NUM_INF_UPPER,                        // Number of segments with `INF` as the upper bound
};

DEFINE_ENUM_CONSTS_NO_EXTRA(IndexStatsColumn, INDEX_STATS_COL, false);

/** @brief Class for logging index statistics */
class IndexStatsLogger : public Logger {
   public:
    /**
     * @brief Write an index statistics entry
     * @param stats The statistics of the index
     * @param length_group_id The ID of the length group within the index
     * @param sub_index_id The ID of the sub-index within the index
     * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
     */
    static void write_entry(const IndexStats &stats, uint length_group_id = 0, uint sub_index_id = 0,
                            bool separate_segment_stats = false);

   private:
    IndexStatsLogger() = default;

    static const str INDEX_STATS_FILE;
};

#endif  // INDEX_STATS_LOGGER_HPP
