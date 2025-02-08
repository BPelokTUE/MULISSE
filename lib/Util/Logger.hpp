#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Search/Options/SearchOptions.hpp"

// ---------------------------------------------------- //
// ----------------- COLUMN ENUMS --------------------- //
// ---------------------------------------------------- //

enum class DatasetSettingsColumn {
    ID,             // Index of the setting within the file
    DATASET_FILE,   // Name to the dataset file
    SERIES_LENGTH,  // Length of each time series
    NUM_CHANNELS,   // Number of channels
    NUM_SERIES,     // Number of time series in the dataset
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetSettingsColumn, DATASET_SETTINGS_COL, false);

/** @brief Enum of the columns of the method settings log file */
enum class IndexSettingsColumn {
    ID,                   // Index of the setting within the file
    DATASET_FILE,         // Name of the indexed dataset file
    INDEX_FILE,           // Path to the index file
    FFTS_FILE,            // Path to the FFTs file, empty if not used
    L_MIN,                // Minimum allowed query length
    L_MAX,                // Maximum allowed query length
    NORMALIZED,           // Whether the query and subsequences are normalized
    INDEX_TYPE,           // Type of index used
    DISTANCE_MEASURE,     // Distance measure used
    SEGMENT_LENGTH,       // Length of the segments for PAA and SAX
    POS_PER_ENV,          // Number of positions per envelope for envelope-based methods
    ISAX_START_CARD,      // Size of the alphabet for the first layer of iSAX indexes
    LEAF_CAPACITY,        // Maximum number of entries in a leaves (if applicable)
    BREAKPOINT_STRATEGY,  // Strategy for getting the breakpoints of the symbol intervals for iSAX indexes
    SPLIT_STRATEGY,       // Strategy for choosing the index to split on for iSAX indexes
    MIN_NUM_BITS_ON_TIE,  // Whether to choose the segment with the minimum number of bits when tied for
                          // EntropyMaximizing split strategy for iSAX indexes
    NUM_BITS_LIMIT,       // Maximum number of bits per segment for iSAX indexes
};

DEFINE_ENUM_CONSTS_NO_EXTRA(IndexSettingsColumn, INDEX_SETTINGS_COL, false);

/** @brief Enum of the columns of the query settings log file */
enum class QuerySettingsColumn {
    ID,                // ID of the setting within the file
    INDEX_FILE,        // Name of the index file used for search (if applicable)
    DATASET_FILE,      // Name of the dataset file used for search
    FFTS_FILE,         // Path to the FFTs file, empty if not used
    QUERY_FILE,        // Path to the query file
    NUM_QUERIES,       // Number of queries
    QUERY_TYPE,        // Type of the query
    R_RANGE_R,         // R parameter for the R-range query
    KNN_K,             // K parameter for the KNN query
    EXACT,             // Whether the search is exact
    NORMALIZED,        // Whether the query and subsequences are normalized
    SEARCH_METHOD,     // Method used for searching
    DISTANCE_MEASURE,  // Distance measure used
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QuerySettingsColumn, QUERY_SETTINGS_COL, false);

/** @brief Enum of the columns of the query log file */
enum class QueryColumn {
    ID,              // ID of the run within the file
    SETTINGS_ID,     // ID of the search settings within the settings file
    QUERY_LENGTH,    // Length of the query
    QUERY_CHANNELS,  // Channels included in the query as a list of ITEM_SEP separated `0`s and `1`s
    QUERY_INDEX,     // The index of the query within the query file (the query file is indicated in the settings file)
    RESULT_SET_TS_INDICES,    // The indices of time series of the entries of the result set, separated by ITEM_SEP
    RESULT_SET_TS_POSITIONS,  // The start positions within their respective time series of the entries of the result
                              // set, separated by ITEM_SEP
    RESULT_SET_DISTANCES,     // The distances of the entries of the result set from the query, separated by ITEM_SEP
    NUM_LEAVES_VISITED,       // Number of leaves visited during the search
    NUM_NODES_VISITED,        // Number of nodes visited during the search
    NUM_TS_EXAMINED,          // Number of time series examined during the search
    TOTAL_TIME,               // Total time taken by the search
    FIRST_LAYER_TIME,         // Time taken to process the first layer in the search
    TREE_TRAVERSAL_TIME,      // Time taken to traverse the tree
    IO_TIME,                  // Time taken to read the time series from the disk
    TS_EXAMINATION_TIME,      // Time taken to examine the time series
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QueryColumn, QUERY_COL, false);

using QC = QueryColumn;

const vec<QC> QUERY_TIME_COLUMNS = {QC::TOTAL_TIME, QC::FIRST_LAYER_TIME, QC::TREE_TRAVERSAL_TIME, QC::IO_TIME,
                                    QC::TS_EXAMINATION_TIME},
              QUERY_COUNT_COLUMNS = {QC::NUM_LEAVES_VISITED, QC::NUM_NODES_VISITED, QC::NUM_TS_EXAMINED},
              QUERY_COLLECTION_COLUMNS = {QC::RESULT_SET_TS_INDICES, QC::RESULT_SET_TS_POSITIONS,
                                          QC::RESULT_SET_DISTANCES, QC::QUERY_CHANNELS},
              QUERY_GENERIC_COLUMNS = {QC::ID, QC::SETTINGS_ID, QC::QUERY_LENGTH, QC::QUERY_INDEX};

// ---------------------------------------------------- //
// ----------------- LOGGER CLASSES ------------------- //
// ---------------------------------------------------- //

class Logger {
   public:
    virtual ~Logger() = default;

   protected:
    /**
     * @brief Determine the index of the new entry in the given file
     *
     * @param file_path The path to the file
     * @return The index of the new entry
     */
    uint determine_index(const str &file_path);

    /**
     * @brief Create a file with the given header if it does not exist
     *
     * @param file_path The path to the file
     * @param header The header of the file
     */
    void file_setup(const str &file_path, const vec<str> &header);

    /**
     * @brief Write a row into the given file stream
     *
     * @tparam C The type of the column enums
     * @param file_path The path to the file
     * @param enum_to_val A map from the column enums to values
     * @param columns A vector defining the order of the columns
     */
    template <typename C>
    void write_row(const str &file_path, const umap<C, str> &enum_to_val, const vec<C> &columns);

    // Separators

    const char COL_SEP = ',', ROW_SEP = '\n', ITEM_SEP = ';';

    // Paths

    const str DATASET_SETTINGS_FILE = "dataset_settings.csv";
    const str INDEX_SETTINGS_FILE = "index_settings.csv";
    const str QUERY_SETTINGS_FILE = "search_settings.csv";
    const str RUN_LOG_FILE = "runs.csv";
};

class DatasetLogger : public Logger {
   public:
    DatasetLogger(const DatasetLogger &) = delete;
    DatasetLogger &operator=(const DatasetLogger &) = delete;

    static void write_entry();

   private:
    DatasetLogger() = default;
};

class IndexLogger : public Logger {
   public:
    IndexLogger() = default;

    IndexLogger &get_instance();

   private:
    // Static
    static IndexLogger instance;
    static bool initialized;
};

class QueryLogger : public Logger {
   public:
    void initialize(const SearchOptions &search_options);

    QueryLogger &get_instance();

    /** @brief Set the given column to the specified value
     * @param col The column to set, expected to be a value from RUN_LOG_GENERIC_COLUMNS
     * @param value The value
     * */
    void set_generic_col(QC col, str value);

    /** @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from RUN_LOG_COUNT_COLUMNS
     * */
    void increment_count_col(QC col);

    /** @brief Start the timer for the given column
     * @param col The column to start the timer for, expected to be a value from RUN_LOG_TIME_COLUMNS
     * */
    void start_timer_for_col(QC col);

    /** @brief Measure the duration of the timer for the given column
     * @param col The column to stop the timer for, expected to be a value from RUN_LOG_TIME_COLUMNS
     * */
    void measure_time_for_col(QC col);

    /** @brief Add an item to the collection column
     * @param col The column to add the item to, expected to be a value from RUN_LOG_COLLECTION_COLUMNS
     * @param item The item to add
     * */
    void add_item_to_collection_col(QC col, str item);

    // ---------------------------------------------------- //

   private:
    std::ifstream m_query_log_ofs;

    umap<QC, str> m_generic_cols;
    umap<QC, uint> m_count_cols;
    umap<QC, double> m_time_cols_start;
    umap<QC, double> m_time_cols_duration;
    umap<QC, vec<str>> m_collection_cols;

    // Static
    static QueryLogger instance;
    static bool initialized;
};

#endif  // LOGGER_HPP
