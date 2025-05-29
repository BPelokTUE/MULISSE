#ifndef INDEX_LOGGER_HPP
#define INDEX_LOGGER_HPP

#include <atomic>

#include "Index/IndexOptions.hpp"
#include "Util/HelperFuncs/Containers.hpp"
#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"

/** @brief Enum of the columns of the index settings log file */
enum class IndexSettingsColumn {
    ID,                           // Index of the setting within the log file
    DATASET_FILE,                 // Name of the indexed dataset file
    INDEX_FILE,                   // Name of the index file
    FFTS_FILE,                    // Name of the FFTs file, empty if not used
    L_MIN,                        // Minimum allowed query length
    L_MAX,                        // Maximum allowed query length
    L_PER_GROUP,                  // Size of length groups
    POS_PER_ENV,                  // Number of positions per envelope for envelope-based methods
    ENTRY_MERGER_TYPE,            // Type of entry merger used
    MERGER_NUM_BITS,              // Number of bits used for SAX-based entry merger, if applicable
    NORMALIZED,                   // Whether the query and subsequences are normalized
    INDEX_TYPE,                   // Type of index used
    LG_SEGMENTATION_STRATEGY,     // Strategy for varying the channel segmentation strategy for different length groups
    CH_SEGMENTATION_STRATEGY,     // Strategy for varying the segmentation strategy for different channels
    SEGMENTATION_STRATEGY,        // Strategy for segmenting the time series channels
    NUM_SEGMENTS,                 // The number of segments per channel used
    MULTI_CHSS_NUM_SEG_FILE,      // The file containing the proportion of segments to use per channel for
                                  // MultiChSegmentationStrategy
    SCORE_BASED_CHSS_SCORE_EXP,   // The exponent used for ScoreToProportionalNumSegments in
                                  // ScoreBasedChSegmentationStrategy
    SAMPLING_CHSS_SEGMENT_LEN,    // The segment length used for SamplingChSegmentationStrategy
    SAMPLING_CHSS_SAMPLE_SIZE,    // The sample size used by SamplingChSegmentationStrategy
    ENV_STATS_CHSS_WEIGHTS_FILE,  // The file containing the weights for IndexStatsScoreFunc in
                                  // EnvStatsChSegmentationStrategy
    ENV_WIDTH_CHSS_MIN_W_UPDATE,  // The minimum sufficient width update for EnvWidthChSegmentationStrategy
    BREAKPOINT_STRATEGY,          // Strategy for getting the breakpoints of the symbol intervals for iSAX indexes
    SPLIT_STRATEGY,               // Strategy for choosing the index to split on for iSAX indexes
    MERGE_IN_LEAVES,              // Whether to merge the entries int the leaves of iSAX indexes
    MIN_NUM_BITS_ON_TIE,          // Whether to choose the segment with the minimum number of bits when tied for
                                  // EntropyMaximizing split strategy for iSAX indexes
    FIRST_LAYER_NUM_BITS,         // Number of bits per segment used in the first layer for iSAX indexes
    LEAF_CAPACITY,                // Maximum number of entries in a leaves (if applicable)
    NUM_BITS_LIMIT,               // Maximum number of bits per segment for iSAX indexes
    ADAPT_TO_DATASET,             // Whether to adapt the index properties to the dataset
    INSERTER_TYPE,                // Type of inserter used for the index
    NUM_LEAVES,                   // Number of leaves in the index
    NUM_NODES,                    // Number of nodes in the index, excluding the root
    NUM_ENTRIES,                  // Number of entries in the index
    INDEXING_TIME_S,              // Time taken to index the dataset in seconds
    SUMMARIZATION_TIME_S,         // Time taken to summarize the subsequences in the dataset in seconds
    INSERTION_TIME_S,             // Time taken to insert the subsequence summaries into the index in seconds
    FFT_CALC_TIME_S,              // Time taken to calculate the FFTs in seconds
    SIZE_ON_DISK_B,               // Size of the index on disk in bytes
    SAMPLE_FRAC,                  // Fraction of the dataset used for indexing, intended for testing
};

using ISC = IndexSettingsColumn;

const vec<ISC> INDEX_TIME_COLUMNS = {ISC::INDEXING_TIME_S, ISC::SUMMARIZATION_TIME_S, ISC::INSERTION_TIME_S,
                                     ISC::FFT_CALC_TIME_S};

const vec<ISC> INDEX_COUNT_COLUMNS = {ISC::NUM_LEAVES, ISC::NUM_NODES, ISC::NUM_ENTRIES, ISC::SIZE_ON_DISK_B};

DEFINE_ENUM_CONSTS_NO_EXTRA(IndexSettingsColumn, INDEX_SETTINGS_COL, false);

// IndexLogger class

/** @brief Class for logging index settings */
class IndexLogger : public Logger {
   public:
    IndexLogger() = default;

    inline static IndexLogger &get_instance() { return instance; };

    static void initialize(const IndexOptions &index_options, Real sample_frac = 1.0);

    /** @brief Write the entry */
    void write_entry();

    /**
     * @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from INDEX_COUNT_COLUMNS
     * @param amount The amount to increment by
     */
    inline void increment_count_col(ISC col, size_t amount = 1) {
        assert(vec_contains(INDEX_COUNT_COLUMNS, col));
        instance.m_count_cols[col] += amount;
    }

    /**
     * @brief Start the timer for the given column
     * @param col The column to start the timer for, expected to be a value from INDEX_TIME_COLUMNS
     */
    inline void start_timer(ISC col) {
        assert(vec_contains(INDEX_TIME_COLUMNS, col));
        m_time_cols_start[col] = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Stop the timer for the given column and save the duration
     * @param col The column to stop the timer for, expected to be a value from INDEX_TIME_COLUMNS
     */
    inline void stop_timer(ISC col) {
        assert(vec_contains(INDEX_TIME_COLUMNS, col));
        auto end = std::chrono::high_resolution_clock::now();
        m_time_cols_duration[col] += std::chrono::duration<double>(end - m_time_cols_start[col]).count();
    }

   private:
    umap<ISC, str> m_columns;
    umap<ISC, std::atomic<size_t>> m_count_cols;
    umap<ISC, TimePoint> m_time_cols_start;
    umap<ISC, double> m_time_cols_duration;
    str m_index_settings_path;

    // Static
    static IndexLogger instance;
    static bool initialized;

    static const str INDEX_SETTINGS_FILE;
};

#endif  // INDEX_LOGGER_HPP
