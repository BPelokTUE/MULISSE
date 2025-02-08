#include <filesystem>

#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

using std::to_string;

// Logger

uint Logger::determine_index(const str &file_path) {
    uint index = 0;
    std::ifstream file_stream(file_path);
    str line;
    std::getline(file_stream, line);  // Skip the header
    while (std::getline(file_stream, line)) ++index;
    return index;
}

void Logger::file_setup(const str &file_path, const vec<str> &header) {
    // If the directory does not exist, create it
    std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());

    if (std::filesystem::exists(file_path)) return;

    std::ofstream ofs(file_path);
    for (uint i = 0; i < header.size(); ++i) {
        ofs << header[i];
        if (i < header.size() - 1) ofs << COL_SEP;
    }
}

template <typename C>
void Logger::write_row(const str &file_path, const umap<C, str> &enum_to_val, const vec<C> &columns) {
    std::ofstream ofs(file_path, std::ios::app);

    ofs << ROW_SEP;
    for (uint i = 0; i < columns.size(); ++i) {
        C col = columns[i];
        ofs << enum_to_val.at(col);
        if (i < columns.size() - 1) ofs << COL_SEP;
    }
}

// DatasetLogger
using DSC = DatasetSettingsColumn;

void DatasetLogger::write_entry() {
    DatasetLogger instance;

    str dataset_settings_path = RunSettings::get_instance().get_logs_path() + instance.DATASET_SETTINGS_FILE;
    instance.file_setup(dataset_settings_path, DATASET_SETTINGS_COL_STRS);

    // Append entry
    uint id = instance.determine_index(dataset_settings_path);
    auto [dataset_file, num_channels, series_len, num_series] = RunSettings::get_instance().get_dataset_props();

    instance.write_row(dataset_settings_path,
                       {
                           {DSC::ID, to_string(id)},
                           {DSC::DATASET_FILE, dataset_file},
                           {DSC::NUM_CHANNELS, to_string(num_channels)},
                           {DSC::SERIES_LENGTH, to_string(series_len)},
                           {DSC::NUM_SERIES, to_string(num_series)},
                       },
                       DATASET_SETTINGS_COL_VALUES);
}

// IndexLogger
IndexLogger IndexLogger::instance = IndexLogger();
bool IndexLogger::initialized = false;
IndexLogger &IndexLogger::get_instance() { return instance; }

// QueryLogger
QueryLogger QueryLogger::instance = QueryLogger();
bool QueryLogger::initialized = false;
QueryLogger &QueryLogger::get_instance() { return instance; }

using QC = QueryColumn;
using QSC = QuerySettingsColumn;

void QueryLogger::initialize(const SearchOptions &search_options) {
    if (initialized) return;
    initialized = true;

    auto &RS = RunSettings::get_instance();

    str query_settings_path = RS.get_logs_path() + QUERY_SETTINGS_FILE;

    // Write settings file
    std::ofstream query_settings_ofs(query_settings_path, std::ios::app);
    for (str str_key : QUERY_SETTINGS_COL_STRS) query_settings_ofs << str_key << instance.COL_SEP;
    query_settings_ofs << instance.ROW_SEP;

    uint settings_id = instance.determine_index(query_settings_path);

    // Get properties from run settings
    MtsNumChannelsT num_channels = RS.get_dataset_props().num_channels;
    auto [query_path, l_min, l_max] = RS.get_query_props();

    // Determine number of queries
    uint num_queries = 0;
    {
        std::ifstream query_stream_read(search_options.query_file);
        str line;
        for (; !query_stream_read.eof(); ++num_queries) std::getline(query_stream_read, line);
        num_queries /= num_channels;
    }
    // Determine query params (r or k)
    DistanceT r_range_r = 0;
    uint knn_k = 0;
    SearchType search_type = search_options.result_set->get_type();
    switch (search_type) {
        case SearchType::R_RANGE:
            r_range_r = static_cast<RRangeResultSet *>(search_options.result_set.get())->get_r();
            break;
        case SearchType::KNN:
            knn_k = static_cast<KnnResultSet *>(search_options.result_set.get())->get_k();
            break;
        default:
            break;
    }

    umap<QSC, str> query_settings_cols({
        {QSC::ID, to_string(settings_id)},
        {QSC::INDEX_FILE, search_options.index_file},
        {QSC::DATASET_FILE, search_options.dataset_file},
        {QSC::FFTS_FILE, RS.m_ffts_path},
        {QSC::QUERY_FILE, query_path},
        {QSC::NUM_QUERIES, to_string(num_queries)},
        {QSC::QUERY_TYPE, SEARCH_TYPE_TO_STR.at(search_type)},
        {QSC::R_RANGE_R, to_string(r_range_r)},
        {QSC::KNN_K, to_string(knn_k)},
        {QSC::EXACT, to_string(search_options.exact)},
        {QSC::NORMALIZED, to_string(search_options.normalized)},
        {QSC::SEARCH_METHOD, SEARCH_METHOD_TYPE_TO_STR.at(search_options.search_method_type)},
        {QSC::DISTANCE_MEASURE, DISTANCE_TYPE_TO_STR.at(search_options.distance_measure->get_type())},
    });

    // Setup for run logging
    for (const auto &col : QUERY_GENERIC_COLUMNS) instance.m_generic_cols[col] = "";
    for (const auto &col : QUERY_COUNT_COLUMNS) instance.m_count_cols[col] = 0;
    for (const auto &col : QUERY_TIME_COLUMNS) {
        instance.m_time_cols_start[col] = 0;
        instance.m_time_cols_duration[col] = 0;
    }
    for (const auto &col : QUERY_COLLECTION_COLUMNS) instance.m_collection_cols[col] = vec<str>();

    str run_log_path = RS.get_logs_path() + RUN_LOG_FILE;
    instance.m_query_log_ofs.open(run_log_path, std::ios::app);
}
