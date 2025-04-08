#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Search/Options/DistanceType.hpp"
#include "Search/Options/SearchType.hpp"
#include "Search/Options/QuerySetOptions.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/Options/IndexOptions.hpp"

using std::to_string;

// ---------------------------------------------------- //
// ----------------- COLUMN ENUMS --------------------- //
// ---------------------------------------------------- //

/** @brief Enum of the columns of the dataset settings log file */
enum class DatasetSettingsColumn {
    ID,             // Index of the setting within the log file
    DATASET_FILE,   // Name to the dataset file
    SERIES_LENGTH,  // Length of each time series
    NUM_CHANNELS,   // Number of channels
    NUM_SERIES,     // Number of time series in the dataset
    SD,             // The standard deviation of the Gaussian noise used for generating the random walk dataset
    SOURCE_CSVS,    // Source CSV files used for generating the CSV dataset
    L_MIN,          // Minimum length of subsequences that will be searched for (required for normalization)
    L_MAX,          // Maximum length of subsequences that will be searched for (required for normalization)
    SEED,           // The random seed to generate the dataset
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetSettingsColumn, DATASET_SETTINGS_COL, false);

/** @brief Enum of the columns of the query set settings log file */
enum class QuerySetSettingsColumn {
    ID,             // Index of the setting within the log file
    DATASET_FILE,   // Name of the dataset file the queries were generated from
    QUERY_FILE,     // Name of the query file
    NUM_QUERIES,    // Number of queries
    L_MIN,          // Minimum length of the queries
    L_MAX,          // Maximum length of the queries
    EXACT_LENGTHS,  // List of exact query lengths used to generate
    USED_CHANNELS,  // Number of channels used for the queries
    CHANNEL_MASK,   // Mask of channels used in the queries
    NOISE,          // The Gaussian noise added to the queries
    SEED,           // The random seed used to generate the queries
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QuerySetSettingsColumn, QUERY_SET_SETTINGS_COL, false);

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

/** @brief Enum of the columns of the search settings log file */
enum class SearchSettingsColumn {
    ID,                   // ID of the setting within the log file
    INDEX_FILE,           // Name of the index file used for search (if applicable)
    DATASET_FILE,         // Name of the dataset file used for search
    FFTS_FILE,            // Name of the FFTs file, empty if not used
    QUERY_FILE,           // Name of the query file
    NUM_QUERIES,          // Number of queries - required for backward compatibility
    QUERY_TYPE,           // Type of the query
    R_RANGE_R,            // R parameter for the R-range query
    KNN_K,                // K parameter for the KNN query
    EXACT,                // Whether the search is exact
    MAX_LEAVES_TO_VISIT,  // Maximum number of leaves to visit
    NORMALIZED,           // Whether the query and subsequences are normalized
    SEARCH_METHOD,        // Method used for searching
    DISTANCE_MEASURE,     // Distance measure used
    EARLY_ABANDONING,     // Whether early abandoning is used (for ED)
    SORT_QUERY,           // Whether the queries are sorted (for ED)
    USE_PRIORITY_QUEUE,   // Whether a priority queue is used (for FlatEnvelopeIndexSearch)
};

DEFINE_ENUM_CONSTS_NO_EXTRA(SearchSettingsColumn, SEARCH_SETTINGS_COL, false);

/** @brief Enum of the columns of the query log file */
enum class QueryColumn {
    ID,                       // ID of the run within the log file
    SETTINGS_ID,              // ID of the search settings within the settings file
    QUERY_ID,                 // ID of the query within the query file
    QUERY_LENGTH,             // Length of the query
    QUERY_CHANNELS,           // Channels included in the query as a list of ITEM_SEP separated `0`s and `1`s
    RESULT_SET_TS_INDICES,    // The indices of time series of the entries of the result set, separated by ITEM_SEP
    RESULT_SET_TS_POSITIONS,  // The start positions within their respective time series of the entries of the result
                              // set, separated by ITEM_SEP
    RESULT_SET_DISTANCES,     // The distances of the entries of the result set from the query, separated by ITEM_SEP
    EXACT_RESULTS,            // Whether the results are known to be exact or not
    NUM_LEAVES_VISITED,       // Number of leaves visited during the search
    NUM_NODES_VISITED,        // Number of nodes visited during the search
    NUM_ENTRIES_EXAMINED,     // Number of index entries examined during the search
    ABANDONING_RATE,          // The rate of early abandoning during the search
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
              QUERY_COUNT_COLUMNS = {QC::NUM_LEAVES_VISITED, QC::NUM_NODES_VISITED, QC::NUM_ENTRIES_EXAMINED},
              QUERY_COLLECTION_COLUMNS = {QC::RESULT_SET_TS_INDICES, QC::RESULT_SET_TS_POSITIONS,
                                          QC::RESULT_SET_DISTANCES, QC::QUERY_CHANNELS},
              QUERY_NUMBER_COLUMNS = {QC::QUERY_ID, QC::QUERY_LENGTH};

// Enums for statistics

#define DEFINE_STAT_COLUMNS(ENUM_SUFFIX) MIN_##ENUM_SUFFIX, MAX_##ENUM_SUFFIX, MEAN_##ENUM_SUFFIX, STD_##ENUM_SUFFIX

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

/** @brief Enum of the columns of the index statistics log file */
enum class IndexStatsColumn {
    INDEX_FILE,                        // Name of the index file
    DEFINE_STAT_COLUMNS(LEAF_SIZE),    // Statistics of the sizes of the leaves / # entries in the leaves
    DEFINE_STAT_COLUMNS(LEAF_HEIGHT),  // Statistics of the height of the leaves
    DEFINE_STAT_COLUMNS(SEG_RANGE),    // Statistics of the range of the segments
    DEFINE_STAT_COLUMNS(SEG_LOWER),    // Statistics of the lower bound of the segments
    DEFINE_STAT_COLUMNS(SEG_UPPER),    // Statistics of the upper bound of the segments
    NUM_INF_LOWER,                     // Number of segments with `-INF` as the lower bound
    NUM_INF_UPPER,                     // Number of segments with `INF` as the upper bound
};

DEFINE_ENUM_CONSTS_NO_EXTRA(IndexStatsColumn, INDEX_STATS_COL, false);

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
            if (enum_to_val.find(col) != enum_to_val.end()) {
                ofs << enum_to_val.at(col);
            } else {
                ofs << "";
            }
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
    const str QUERY_SET_SETTINGS_FILE = "query_set_settings.csv";
    const str QUERY_STATS_FILE = "query_stats.csv";
    const str INDEX_SETTINGS_FILE = "index_settings.csv";
    const str INDEX_STATS_FILE = "index_stats.csv";
    const str SEARCH_SETTINGS_FILE = "search_settings.csv";
    const str RUN_LOG_FILE = "runs.csv";
};

enum DatasetType { RANDOM_WALK, CSV };

struct IDatasetLogAttributes {
    virtual ~IDatasetLogAttributes() = default;

    virtual DatasetType get_type() = 0;
};

struct RandomWalkLogAttributes : IDatasetLogAttributes {
    RandomWalkLogAttributes(Real noise, int seed);

    DatasetType get_type() override;

    Real noise;
    int seed;
};

struct CsvDatasetLogAttributes : IDatasetLogAttributes {
    CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint l_min, uint l_max, int seed);

    DatasetType get_type() override;

    vec<str> source_csvs;
    uint series_generated, l_min, l_max;
    int seed;
};

/** @brief Class for logging dataset settings */
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

/** @brief Class for logging query set settings */
class QuerySetLogger : public Logger {
   public:
    QuerySetLogger(const QuerySetLogger &) = delete;
    QuerySetLogger &operator=(const QuerySetLogger &) = delete;

    /**
     * @brief Write the entry
     * @param opts Options used to generate the query set
     */
    static void write_entry(QuerySetOptions &opts);

   private:
    template <typename T>
    str get_num_vec_str(const vec<T> &values) {
        str result_str = "";
        for (uint i = 0; i < values.size(); ++i) {
            result_str += to_string(values[i]);
            if (i < values.size() - 1) result_str += ITEM_SEP;
        }
        return result_str;
    }

    QuerySetLogger() = default;
};

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

/** Class for logging search settings and query results */
class QueryLogger : public Logger {
   public:
    static void initialize(const SearchOptions &search_options);

    inline static QueryLogger &get_instance() { return instance; }

    /**
     * @brief Set the given column to the specified value
     * @tparam T The type of the value
     * @param col The column to set, expected to be a value from QUERY_NUMBER_COLUMNS
     * @param value The value
     * */
    template <typename T>
    inline void set_number_col(QC col, T value) {
        assert(vec_contains(QUERY_NUMBER_COLUMNS, col));
        instance.m_settable_cols[col] = std::to_string(value);
    }

    /**
     * @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from QUERY_COUNT_COLUMNS
     * @param amount The amount to increment by
     * */
    inline void increment_count_col(QC col, uint amount = 1) {
        assert(vec_contains(QUERY_COUNT_COLUMNS, col));
        instance.m_count_cols[col] += amount;
    }

    /**
     * @brief Increment the number of points in the entries examined
     * @param amount The amount to increment by
     * */
    inline void increment_num_points_in_examined_entries(uint64_t amount) { num_points_in_examined_entries += amount; }

    /**
     * @brief Increment the number of points examined
     * @param amount The amount to increment by
     */
    inline void increment_num_points_examined(uint64_t amount) { num_points_examined += amount; }

    /**
     * @brief Start the timer for the given column
     * @param col The column to start the timer for, expected to be a value from QUERY_TIME_COLUMNS
     */
    inline void start_timer(QC col) {
        assert(vec_contains(QUERY_TIME_COLUMNS, col));
        instance.m_time_cols_start[col] = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Stop the timer for the given column and save the duration
     * @param col The column to stop the timer for, expected to be a value from QUERY_TIME_COLUMNS
     */
    inline void stop_timer(QC col) {
        assert(vec_contains(QUERY_TIME_COLUMNS, col));
        auto end = std::chrono::high_resolution_clock::now();
        instance.m_time_cols_duration[col] +=
            std::chrono::duration<double>(end - instance.m_time_cols_start[col]).count();
    }

    /**
     * @brief Log information about the query into the current run entry
     * @param query The query to log
     */
    void log_query(const vec<vec<Real>> &query);

    /**
     * @brief Log information about the results of the search into the current run entry
     * @param results The results of the search
     */
    void log_results(const SearchResults &results);

    /** @brief Reset the current run entry */
    void reset_entry();

    /** @brief Write the current run entry */
    void write_entry();

    // ---------------------------------------------------- //

   private:
    str get_collection_str(QC col);

    std::ifstream m_query_log_ofs;
    str m_search_settings_id_str;

    umap<QC, str> m_settable_cols;
    umap<QC, uint> m_count_cols;
    umap<QC, TimePoint> m_time_cols_start;
    umap<QC, double> m_time_cols_duration;
    umap<QC, vec<str>> m_collection_cols;

    __uint128_t num_points_in_examined_entries = 0, num_points_examined = 0;

    // Static
    static QueryLogger instance;
    static bool initialized;
};

// Statistics

struct AttributeStats {
    Real min, max, mean, st_dev, sum, sum_sq;

    AttributeStats();

    void update(Real value);

    void update(Real value, size_t count);

    void calculate(uint count);
};

struct QueryStats {
    AttributeStats dist_stats;
    size_t subs_count = 0;
    Real rc_using_max, rc_using_mean;

    QueryStats() = default;

    void calculate();
};

struct IndexStats {
    AttributeStats leaf_size_stats;
    AttributeStats leaf_height_stats;
    AttributeStats seg_range_stats;
    AttributeStats seg_lower_stats;
    AttributeStats seg_upper_stats;

    size_t leaf_count = 0, seg_count = 0;
    size_t num_inf_lower = 0, num_inf_upper = 0;

    IndexStats() = default;

    void update_leaf_stats(Real fill, Real height);

    void update_seg_stats(Real lower, Real upper, size_t count = 1);

    void calculate();
};

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
};

/** @brief Class for logging index statistics */
class IndexStatsLogger : public Logger {
   public:
    /**
     * @brief Write an index statistics entry
     * @param stats The statistics of the index
     */
    static void write_entry(const IndexStats &stats);

   private:
    IndexStatsLogger() = default;
};

#endif  // LOGGER_HPP
