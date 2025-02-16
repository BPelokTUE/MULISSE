#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/Options/IndexOptions.hpp"

using std::to_string;

// ---------------------------------------------------- //
// ----------------- COLUMN ENUMS --------------------- //
// ---------------------------------------------------- //

enum class DatasetSettingsColumn {
    ID,             // Index of the setting within the file
    DATASET_FILE,   // Name to the dataset file
    SERIES_LENGTH,  // Length of each time series
    NUM_CHANNELS,   // Number of channels
    NUM_SERIES,     // Number of time series in the dataset
    SD,             // The standard deviation of the Gaussian noise used for generating the random walk dataset
    SOURCE_CSVS,    // Source CSV files used for generating the CSV dataset
    LOW_SD_LEN,     // Length of the subsequence with low standard deviation that causes the time series to be
                    // discarded
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetSettingsColumn, DATASET_SETTINGS_COL, false);

/** @brief Enum of the columns of the method settings log file */
enum class IndexSettingsColumn {
    ID,                    // Index of the setting within the file
    DATASET_FILE,          // Name of the indexed dataset file
    INDEX_FILE,            // Path to the index file
    FFTS_FILE,             // Path to the FFTs file, empty if not used
    L_MIN,                 // Minimum allowed query length
    L_MAX,                 // Maximum allowed query length
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
    NUM_LEAVES,            // Number of leaves in the index
    NUM_NODES,             // Number of nodes in the index, excluding the root
    INDEXING_TIME_S,       // Time taken to index the dataset in seconds
    FFT_CALC_TIME_S,       // Time taken to calculate the FFTs in seconds
};

using ISC = IndexSettingsColumn;

const vec<ISC> INDEX_TIME_COLUMNS = {ISC::INDEXING_TIME_S, ISC::FFT_CALC_TIME_S};

const vec<ISC> INDEX_COUNT_COLUMNS = {ISC::NUM_LEAVES, ISC::NUM_NODES};

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
    EARLY_ABANDONING,  // Whether early abandoning is used (for ED)
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QuerySettingsColumn, QUERY_SETTINGS_COL, false);

/** @brief Enum of the columns of the query log file */
enum class QueryColumn {
    ID,                       // ID of the run within the file
    SETTINGS_ID,              // ID of the search settings within the settings file
    QUERY_ID,                 // ID of the query within the query file
    QUERY_LENGTH,             // Length of the query
    QUERY_CHANNELS,           // Channels included in the query as a list of ITEM_SEP separated `0`s and `1`s
    RESULT_SET_TS_INDICES,    // The indices of time series of the entries of the result set, separated by ITEM_SEP
    RESULT_SET_TS_POSITIONS,  // The start positions within their respective time series of the entries of the result
                              // set, separated by ITEM_SEP
    RESULT_SET_DISTANCES,     // The distances of the entries of the result set from the query, separated by ITEM_SEP
    NUM_LEAVES_VISITED,       // Number of leaves visited during the search
    NUM_NODES_VISITED,        // Number of nodes visited during the search
    NUM_TS_EXAMINED,          // Number of time series examined during the search
    TOTAL_TIME_S,             // Total time taken by the search in seconds
    FIRST_LAYER_TIME_S,       // Time taken to process the first layer in the search in seconds
    TREE_TRAVERSAL_TIME_S,    // Time taken to traverse the tree in seconds
    IO_TIME_S,                // Time taken to read the time series from the disk in seconds
    TS_EXAMINATION_TIME_S,    // Time taken to examine the time series in seconds
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QueryColumn, QUERY_COL, false);

using QC = QueryColumn;

const vec<QC> QUERY_TIME_COLUMNS = {QC::TOTAL_TIME_S, QC::FIRST_LAYER_TIME_S, QC::TREE_TRAVERSAL_TIME_S, QC::IO_TIME_S,
                                    QC::TS_EXAMINATION_TIME_S},
              QUERY_COUNT_COLUMNS = {QC::NUM_LEAVES_VISITED, QC::NUM_NODES_VISITED, QC::NUM_TS_EXAMINED},
              QUERY_COLLECTION_COLUMNS = {QC::RESULT_SET_TS_INDICES, QC::RESULT_SET_TS_POSITIONS,
                                          QC::RESULT_SET_DISTANCES, QC::QUERY_CHANNELS},
              QUERY_NUMBER_COLUMNS = {QC::QUERY_ID, QC::QUERY_LENGTH};

// ---------------------------------------------------- //
// ----------------- LOGGER CLASSES ------------------- //
// ---------------------------------------------------- //

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class Logger {
   public:
    virtual ~Logger() = default;

   protected:
    /**
     * @brief Determine the index of the new entry in the given file
     * @param file_path The path to the file
     * @return The index of the new entry
     */
    uint determine_index(const str &file_path);

    /**
     * @brief Create a file with the given header if it does not exist
     * @param file_path The path to the file
     * @param header The header of the file
     */
    void file_setup(const str &file_path, const vec<str> &header);

    /**
     * @brief Write a row into the given file stream
     * @tparam C The type of the column enums
     * @param file_path The path to the file
     * @param enum_to_val A map from the column enums to values
     * @param columns A vector defining the order of the columns
     */
    template <typename C>
    void write_row(const str &file_path, const umap<C, str> &enum_to_val, const vec<C> &columns) {
#ifndef DISABLE_LOGGING
        std::ofstream ofs(file_path, std::ios::app);

        ofs << ROW_SEP;
        for (uint i = 0; i < columns.size(); ++i) {
            C col = columns[i];
            ofs << enum_to_val.at(col);
            if (i < columns.size() - 1) ofs << COL_SEP;
        }
#endif
    }

    /**
     * @brief Get the string representation of the given number, or emtpy string if the number is zero
     * @tparam T The type of the number
     * @param num The number
     * @return The string representation of the number, or empty string if the number is zero
     */
    template <typename T>
    static str format_num_param(T num) {
        return num == 0 ? "" : to_string(num);
    }

    // Separators

    const char COL_SEP = ',', ROW_SEP = '\n', ITEM_SEP = ';';

    // Paths

    const str DATASET_SETTINGS_FILE = "dataset_settings.csv";
    const str INDEX_SETTINGS_FILE = "index_settings.csv";
    const str QUERY_SETTINGS_FILE = "search_settings.csv";
    const str RUN_LOG_FILE = "runs.csv";
};

enum DatasetType { RANDOM_WALK, CSV };

struct IDatasetLogAttributes {
    virtual ~IDatasetLogAttributes() = default;

    virtual DatasetType get_type() = 0;
};

struct RandomWalkLogAttributes : IDatasetLogAttributes {
    RandomWalkLogAttributes(float noise);

    DatasetType get_type() override;

    float noise;
};

struct CsvDatasetLogAttributes : IDatasetLogAttributes {
    CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint low_sd_len);

    DatasetType get_type() override;

    vec<str> source_csvs;
    uint series_generated;
    uint low_sd_len;
};
class DatasetLogger : public Logger {
   public:
    DatasetLogger(const DatasetLogger &) = delete;
    DatasetLogger &operator=(const DatasetLogger &) = delete;

    /**
     * @brief Write the entry
     * @param attributes Attributes of the generated dataset
     * */
    static void write_entry(uptr<IDatasetLogAttributes> attributes);

   private:
    DatasetLogger() = default;
};

class IndexLogger : public Logger {
   public:
    IndexLogger() = default;

    static IndexLogger &get_instance();

    static void initialize(const IndexOptions &index_options);

    /** @brief Write the entry */
    void write_entry();

    /**
     * @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from INDEX_COUNT_COLUMNS
     * @param amount The amount to increment by
     */
    void increment_count_col(ISC col, uint amount = 1);

    /**
     * @brief Start the timer for the given column
     * @param col The column to start the timer for, expected to be a value from INDEX_TIME_COLUMNS
     */
    void start_timer(ISC col);

    /**
     * @brief Stop the timer for the given column and save the duration
     * @param col The column to stop the timer for, expected to be a value from INDEX_TIME_COLUMNS
     */
    void stop_timer(ISC col);

   private:
    umap<ISC, str> m_columns;
    umap<ISC, uint> m_count_cols;
    umap<ISC, TimePoint> m_time_cols_start;
    umap<ISC, double> m_time_cols_duration;
    str m_index_settings_path;

    // Static
    static IndexLogger instance;
    static bool initialized;
};

class QueryLogger : public Logger {
   public:
    static void initialize(const SearchOptions &search_options);

    static QueryLogger &get_instance();

    /**
     * @brief Set the given column to the specified value
     * @tparam T The type of the value
     * @param col The column to set, expected to be a value from QUERY_NUMBER_COLUMNS
     * @param value The value
     * */
    template <typename T>
    void set_number_col(QC col, T value) {
        assert(vec_contains(QUERY_NUMBER_COLUMNS, col));
        instance.m_settable_cols[col] = std::to_string(value);
    }

    /**
     * @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from QUERY_COUNT_COLUMNS
     * */
    void increment_count_col(QC col);

    /**
     * @brief Start the timer for the given column
     * @param col The column to start the timer for, expected to be a value from QUERY_TIME_COLUMNS
     */
    void start_timer(QC col);

    /**
     * @brief Stop the timer for the given column and save the duration
     * @param col The column to stop the timer for, expected to be a value from QUERY_TIME_COLUMNS
     */
    void stop_timer(QC col);

    /**
     * @brief Log information about the query into the current run entry
     * @param query The query to log
     */
    void log_query(const vec<vec<float>> &query);

    /**
     * @brief Log information about the results of the search into the current run entry
     * @param results The results of the search
     */
    void log_results(const vec<SearchResult> &results);

    /** @brief Reset the current run entry */
    void reset_entry();

    /** @brief Write the current run entry */
    void write_entry();

    // ---------------------------------------------------- //

   private:
    str get_collection_str(QC col);

    std::ifstream m_query_log_ofs;
    str m_query_settings_id_str;

    umap<QC, str> m_settable_cols;
    umap<QC, uint> m_count_cols;
    umap<QC, TimePoint> m_time_cols_start;
    umap<QC, double> m_time_cols_duration;
    umap<QC, vec<str>> m_collection_cols;

    // Static
    static QueryLogger instance;
    static bool initialized;
};

#endif  // LOGGER_HPP
