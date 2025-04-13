#ifndef INDEX_LOGGER_HPP
#define INDEX_LOGGER_HPP

#include "Util/Logging/Logger.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enum of the columns of the index settings log file */
enum class IndexSettingsColumn {
    ID,                    // Index of the setting within the log file
    DATASET_FILE,          // Name of the indexed dataset file
    INDEX_FILE,            // Name of the index file
    FFTS_FILE,             // Name of the FFTs file, empty if not used
    L_MIN,                 // Minimum allowed query length
    L_MAX,                 // Maximum allowed query length
    L_PER_GROUP,           // Size of length groups
    NORMALIZED,            // Whether the query and subsequences are normalized
    INDEX_TYPE,            // Type of index used
    SEGMENT_LENGTH,        // Length of the segments for PAA and SAX
    POS_PER_ENV,           // Number of positions per envelope for envelope-based methods
    FIRST_LAYER_NUM_BITS,  // Number of bits per segment used in the first layer for iSAX indexes
    LEAF_CAPACITY,         // Maximum number of entries in a leaves (if applicable)
    BREAKPOINT_STRATEGY,   // Strategy for getting the breakpoints of the symbol intervals for iSAX indexes
    SPLIT_STRATEGY,        // Strategy for choosing the index to split on for iSAX indexes
    MIN_NUM_BITS_ON_TIE,   // Whether to choose the segment with the minimum number of bits when tied for
                           // EntropyMaximizing split strategy for iSAX indexes
    NUM_BITS_LIMIT,        // Maximum number of bits per segment for iSAX indexes
    ADAPT_TO_DATASET,      // Whether to adapt the index properties to the dataset
    INSERTER_TYPE,         // Type of inserter used for the index
    NUM_LEAVES,            // Number of leaves in the index
    NUM_NODES,             // Number of nodes in the index, excluding the root
    NUM_ENTRIES,           // Number of entries in the index
    INDEXING_TIME_S,       // Time taken to index the dataset in seconds
    SUMMARIZATION_TIME_S,  // Time taken to summarize the subsequences in the dataset in seconds
    INSERTION_TIME_S,      // Time taken to insert the subsequence summaries into the index in seconds
    FFT_CALC_TIME_S,       // Time taken to calculate the FFTs in seconds
};

using ISC = IndexSettingsColumn;

const vec<ISC> INDEX_TIME_COLUMNS = {ISC::INDEXING_TIME_S, ISC::SUMMARIZATION_TIME_S, ISC::INSERTION_TIME_S,
                                     ISC::FFT_CALC_TIME_S};

const vec<ISC> INDEX_COUNT_COLUMNS = {ISC::NUM_LEAVES, ISC::NUM_NODES, ISC::NUM_ENTRIES};

DEFINE_ENUM_CONSTS_NO_EXTRA(IndexSettingsColumn, INDEX_SETTINGS_COL, false);

// IndexLogger class

/** @brief Class for logging index settings */
class IndexLogger : public Logger {
   public:
    IndexLogger() = default;

    inline static IndexLogger &get_instance() { return instance; };

    static void initialize(const IndexOptions &index_options);

    /** @brief Write the entry */
    void write_entry();

    /**
     * @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from INDEX_COUNT_COLUMNS
     * @param amount The amount to increment by
     */
    inline void increment_count_col(ISC col, uint amount = 1) {
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
    umap<ISC, std::atomic<uint>> m_count_cols;
    umap<ISC, TimePoint> m_time_cols_start;
    umap<ISC, double> m_time_cols_duration;
    str m_index_settings_path;

    // Static
    static IndexLogger instance;
    static bool initialized;
};

#endif  // INDEX_LOGGER_HPP
