#include "Util/Logging/QueryLogger.hpp"

#include <sstream>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

QueryLogger QueryLogger::instance = QueryLogger();
bool QueryLogger::initialized = false;

using QC = QueryColumn;
using SSC = SearchSettingsColumn;

void QueryLogger::initialize(const SearchOptions &search_options) {
    if (initialized) return;
    initialized = true;

    auto &RS = RunSettings::get_instance();

    str search_settings_path = fs::path(RS.get_logs_path()) / instance.SEARCH_SETTINGS_FILE;

    // Write settings file
    instance.file_setup(search_settings_path, SEARCH_SETTINGS_COL_STRS);

    instance.m_search_settings_id_str = to_string(instance.determine_index(search_settings_path));
    // Determine number of queries
    uint num_queries = 0;
    {
        std::ifstream query_stream_read(RS.get_query_path());
        str line;
        for (; !query_stream_read.eof(); ++num_queries) std::getline(query_stream_read, line);
        num_queries /= RS.m_dataset_props.m_num_channels;
    }
    // Determine query params (r or k)
    Real r_range_r = search_options.m_search_type == SearchType::R_RANGE ? search_options.m_r_range_r : 0;
    uint knn_k = search_options.m_search_type == SearchType::KNN ? search_options.m_knn_k : 0;

    str early_abandon_str = "";
    if (search_options.m_distance_type == DistanceType::ED)
        early_abandon_str = to_string(search_options.m_use_early_abandoning);

    str sort_query_str = "";
    if (search_options.m_distance_type == DistanceType::ED) sort_query_str = to_string(search_options.m_sort_queries);

    str use_priority_queue_str = "";
    if (arr_contains(METHODS_W_FLAT_PART, search_options.m_search_method_type))
        use_priority_queue_str = to_string(search_options.m_use_priority_queue);

    instance.write_row(search_settings_path,
                       {
                           {SSC::ID, instance.m_search_settings_id_str},
                           {SSC::INDEX_FILE, RS.m_index_file},
                           {SSC::DATASET_FILE, RS.m_dataset_props.m_file},
                           {SSC::FFTS_FILE, RS.m_ffts_file},
                           {SSC::QUERY_FILE, RS.m_query_file},
                           {SSC::NUM_QUERIES, to_string(num_queries)},
                           {SSC::QUERY_TYPE, SEARCH_TYPE_TO_STR.at(search_options.m_search_type)},
                           {SSC::R_RANGE_R, format_num_param(r_range_r)},
                           {SSC::KNN_K, format_num_param(knn_k)},
                           {SSC::EXACT, to_string(search_options.m_exact)},
                           {SSC::MAX_LEAVES_TO_VISIT, format_num_param(search_options.m_max_leaves_to_visit)},
                           {SSC::NORMALIZED, to_string(search_options.m_normalized)},
                           {SSC::SEARCH_METHOD, SEARCH_METHOD_TYPE_TO_STR.at(search_options.m_search_method_type)},
                           {SSC::DISTANCE_MEASURE, DISTANCE_TYPE_TO_STR.at(search_options.m_distance_type)},
                           {SSC::EARLY_ABANDONING, early_abandon_str},
                           {SSC::SORT_QUERY, sort_query_str},
                           {SSC::USE_PRIORITY_QUEUE, use_priority_queue_str},
                       },
                       SEARCH_SETTINGS_COL_ENUMS);

    // Setup for run logging
    instance.reset_entry();
    str run_log_path = fs::path(RS.get_logs_path()) / instance.RUN_LOG_FILE;
    instance.file_setup(run_log_path, QUERY_COL_STRS);
    instance.m_query_log_ofs.open(run_log_path, std::ios::app);
}

void QueryLogger::reset_entry() {
    for (const auto &col : QUERY_NUMBER_COLUMNS) instance.m_settable_cols[col] = "";
    for (const auto &col : QUERY_COUNT_COLUMNS) instance.m_count_cols[col] = 0;
    for (const auto &col : QUERY_TIME_COLUMNS) {
        instance.m_time_cols_start[col] = TimePoint();
        instance.m_time_cols_duration[col] = 0;
    }
    for (const auto &col : QUERY_COLLECTION_COLUMNS) instance.m_collection_cols[col] = vec<str>();
}

void QueryLogger::log_query(const vec<vec<Real>> &query) {
    size_t query_len = 0;
    vec<str> included;

    for (const auto &channel : query) {
        included.push_back(channel.empty() ? "0" : "1");
        query_len = std::max(query_len, channel.size());
    }
    instance.m_settable_cols[QC::QUERY_LENGTH] = to_string(query_len);
    instance.m_collection_cols[QC::QUERY_CHANNELS] = included;
}

void QueryLogger::log_results(const SearchResults &results) {
    for (auto result : results.m_results) {
        auto [ts_index, ts_position, ts_length] = result.m_subs_info;
        instance.m_collection_cols[QC::RESULT_SET_TS_INDICES].push_back(to_string(ts_index));
        instance.m_collection_cols[QC::RESULT_SET_TS_POSITIONS].push_back(to_string(ts_position));
        instance.m_collection_cols[QC::RESULT_SET_DISTANCES].push_back(to_string(std::sqrt(result.m_distance)));
    }
    instance.m_settable_cols[QC::EXACT_RESULTS] = to_string(results.m_exact);
}

str uint128_to_str(const __uint128_t &value) {
    std::stringstream ss;
    ss << static_cast<uint64_t>(value >> 64) << static_cast<uint64_t>(value & 0xFFFFFFFFFFFFFFFF);
    return ss.str();
};

void QueryLogger::write_entry() {
    auto &RS = RunSettings::get_instance();
    str run_log_path = fs::path(RS.get_logs_path()) / instance.RUN_LOG_FILE;
    umap<QC, str> columns({
        {QC::ID, to_string(instance.determine_index(run_log_path))},
        {QC::SETTINGS_ID, m_search_settings_id_str},
        {QC::EXACT_RESULTS, m_settable_cols[QC::EXACT_RESULTS]},
    });

    for (const auto &col : QUERY_NUMBER_COLUMNS) columns[col] = m_settable_cols[col];
    for (const auto &col : QUERY_COUNT_COLUMNS) columns[col] = to_string(m_count_cols[col]);
    for (const auto &col : QUERY_TIME_COLUMNS) columns[col] = to_string(m_time_cols_duration[col]);
    for (const auto &col : QUERY_COLLECTION_COLUMNS)
        columns[col] = instance.get_collection_str(instance.m_collection_cols[col]);

    columns[QC::NUM_PTS_IN_EXAMINED_ENTRIES] = uint128_to_str(m_num_points_in_examined_entries);
    columns[QC::NUM_PTS_EXAMINED] = uint128_to_str(m_num_points_examined);

    bool abandoning_used = m_num_points_examined < m_num_points_in_examined_entries;
    columns[QC::ABANDONING_RATE] =
        to_string(abandoning_used ? 1.0 - R(m_num_points_examined) / R(m_num_points_in_examined_entries) : 0.0);

    uint num_series = RS.get_dataset_props().m_num_series;
    uint series_len = RS.get_dataset_props().m_series_len;
    uint query_len = U(std::stoul(m_settable_cols[QC::QUERY_LENGTH]));
    size_t subs_in_dataset = static_cast<size_t>(num_series * (series_len - query_len + 1));

    columns[QC::PRUNING_RATIO] = to_string(1.0 - R(m_count_cols[QC::NUM_SUBS_EXAMINED]) / R(subs_in_dataset));

    write_row(run_log_path, columns, QUERY_COL_ENUMS);
}
