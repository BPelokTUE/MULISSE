#ifndef QUERY_LOGGER_HPP
#define QUERY_LOGGER_HPP

#include "Search/Results/SearchResult.hpp"
#include "Search/SearchOptions.hpp"
#include "Util/HelperFuncs/Containers.hpp"
#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"

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
    EXAMINE_WHOLE,        // Whether to examine the whole time series when distance calculation is performed
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
    NUM_MIN_DIST_CALCULATED,  // Number of minimum distance calculations performed during the search
    NUM_PTS_IN_EXAMINED_ENTRIES,  // Number of points in the examined entries
    NUM_PTS_EXAMINED,             // Number of points examined during the search
    NUM_SUBS_EXAMINED,            // Number of subsequences examined during the search
    MIN_DIST_TOTAL,               // TODO: only for testing, remove later
    PRUNING_RATIO,       // One minus the # examined subsequences over the total # of subsequences of the query length
    ABANDONING_RATE,     // The rate of early abandoning during the search
    TOTAL_TIME_S,        // Total time taken by the search in seconds
    FIRST_LAYER_TIME_S,  // Time taken to process the first layer in the search in seconds
    TREE_TRAVERSAL_TIME_S,  // Time taken to traverse the tree in seconds
    IO_TIME_S,              // Time taken to read the time series from the disk in seconds
    TS_EXAMINATION_TIME_S,  // Time taken to examine the time series in seconds
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QueryColumn, QUERY_COL, false);

using QC = QueryColumn;

constexpr std::array QUERY_TIME_COLUMNS = {QC::TOTAL_TIME_S, QC::FIRST_LAYER_TIME_S, QC::TREE_TRAVERSAL_TIME_S,
                                           QC::IO_TIME_S, QC::TS_EXAMINATION_TIME_S};
constexpr std::array QUERY_COUNT_COLUMNS = {QC::NUM_LEAVES_VISITED, QC::NUM_NODES_VISITED, QC::NUM_ENTRIES_EXAMINED,
                                            QC::NUM_MIN_DIST_CALCULATED, QC::NUM_SUBS_EXAMINED};
constexpr std::array QUERY_COLLECTION_COLUMNS = {QC::RESULT_SET_TS_INDICES, QC::RESULT_SET_TS_POSITIONS,
                                                 QC::RESULT_SET_DISTANCES, QC::QUERY_CHANNELS};
constexpr std::array QUERY_NUMBER_COLUMNS = {QC::QUERY_ID, QC::QUERY_LENGTH, QC::MIN_DIST_TOTAL};

// QueryLogger class

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
        assert(arr_contains(QUERY_NUMBER_COLUMNS, col));
        instance.m_settable_cols[col] = std::to_string(value);
    }

    /**
     * @brief Increment the value of the given column
     * @param col The column to increment, expected to be a value from QUERY_COUNT_COLUMNS
     * @param amount The amount to increment by
     * */
    inline void increment_count_col(QC col, uint amount = 1) {
        assert(arr_contains(QUERY_COUNT_COLUMNS, col));
        instance.m_count_cols[col] += amount;
    }

    /**
     * @brief Increment the number of points in the entries examined
     * @param amount The amount to increment by
     * */
    inline void increment_num_points_in_examined_entries(uint64_t amount) {
        m_num_points_in_examined_entries += amount;
    }

    /**
     * @brief Increment the number of points examined
     * @param amount The amount to increment by
     */
    inline void increment_num_points_examined(uint64_t amount) { m_num_points_examined += amount; }

    /**
     * @brief Start the timer for the given column
     * @param col The column to start the timer for, expected to be a value from QUERY_TIME_COLUMNS
     */
    inline void start_timer(QC col) {
        assert(arr_contains(QUERY_TIME_COLUMNS, col));
        instance.m_time_cols_start[col] = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Stop the timer for the given column and save the duration
     * @param col The column to stop the timer for, expected to be a value from QUERY_TIME_COLUMNS
     */
    inline void stop_timer(QC col) {
        assert(arr_contains(QUERY_TIME_COLUMNS, col));
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
    std::ifstream m_query_log_ofs;
    str m_search_settings_id_str;

    umap<QC, str> m_settable_cols;
    umap<QC, uint> m_count_cols;
    umap<QC, TimePoint> m_time_cols_start;
    umap<QC, double> m_time_cols_duration;
    umap<QC, vec<str>> m_collection_cols;

    __uint128_t m_num_points_in_examined_entries = 0, m_num_points_examined = 0;

    // Static
    static QueryLogger instance;
    static bool initialized;

    static const str SEARCH_SETTINGS_FILE;
    static const str RUN_LOG_FILE;
};

#endif  // QUERY_LOGGER_HPP
