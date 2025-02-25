#include <filesystem>
#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"
#include "Search/DistanceMeasure.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/Options/IndexOptions.hpp"

using std::to_string;

namespace fs = std::filesystem;

// Logger

uint Logger::determine_index(const str &file_path) {
    uint index = 0;
#ifndef DISABLE_LOGGING
    std::ifstream file_stream(file_path);
    str line;
    std::getline(file_stream, line);  // Skip the header
    while (std::getline(file_stream, line)) ++index;
#endif
    return index;
}

void Logger::file_setup(const str &file_path, const vec<str> &header) {
#ifndef DISABLE_LOGGING
    // If the directory does not exist, create it
    std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());

    if (std::filesystem::exists(file_path)) return;

    std::ofstream ofs(file_path);
    for (uint i = 0; i < header.size(); ++i) {
        ofs << header[i];
        if (i < header.size() - 1) ofs << COL_SEP;
    }
#endif
}

// DatasetLogger

RandomWalkLogAttributes::RandomWalkLogAttributes(float noise) : noise(noise) {}

DatasetType RandomWalkLogAttributes::get_type() { return RANDOM_WALK; }

CsvDatasetLogAttributes::CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint low_sd_len)
    : source_csvs(source_csvs), series_generated(series_generated), low_sd_len(low_sd_len) {}

DatasetType CsvDatasetLogAttributes::get_type() { return CSV; }

using DSC = DatasetSettingsColumn;

void DatasetLogger::write_entry(uptr<IDatasetLogAttributes> attributes) {
#ifndef DISABLE_LOGGING
    DatasetLogger instance;

    str dataset_settings_path = fs::path(RunSettings::get_instance().get_logs_path()) / instance.DATASET_SETTINGS_FILE;
    instance.file_setup(dataset_settings_path, DATASET_SETTINGS_COL_STRS);

    // Append entry
    uint id = instance.determine_index(dataset_settings_path);
    auto [dataset_file, num_channels, series_len, num_series] = RunSettings::get_instance().get_dataset_props();

    str sd_str = "", source_csv_str = "", low_sd_len_str = "";
    switch (attributes->get_type()) {
        case RANDOM_WALK: {
            auto *rw_attributes = static_cast<RandomWalkLogAttributes *>(attributes.get());
            sd_str = to_string(rw_attributes->noise);
            break;
        }
        case CSV: {
            auto *csv_attributes = static_cast<CsvDatasetLogAttributes *>(attributes.get());
            num_series = csv_attributes->series_generated;
            const vec<str> &source_csvs = csv_attributes->source_csvs;
            num_channels = source_csvs.size();
            for (uint i = 0; i < source_csvs.size(); ++i) {
                source_csv_str += source_csvs[i];
                if (i < source_csvs.size() - 1) source_csv_str += instance.ITEM_SEP;
            }
            low_sd_len_str = to_string(csv_attributes->low_sd_len);
            break;
        }
    }

    instance.write_row(dataset_settings_path,
                       {
                           {DSC::ID, to_string(id)},
                           {DSC::DATASET_FILE, dataset_file},
                           {DSC::NUM_CHANNELS, to_string(num_channels)},
                           {DSC::SERIES_LENGTH, to_string(series_len)},
                           {DSC::NUM_SERIES, to_string(num_series)},
                           {DSC::SD, sd_str},
                           {DSC::SOURCE_CSVS, source_csv_str},
                           {DSC::LOW_SD_LEN, low_sd_len_str},
                       },
                       DATASET_SETTINGS_COL_ENUMS);
#endif
}

// IndexLogger
IndexLogger IndexLogger::instance = IndexLogger();
bool IndexLogger::initialized = false;
IndexLogger &IndexLogger::get_instance() {
    assert(initialized);
    return instance;
}

using ISC = IndexSettingsColumn;

void IndexLogger::initialize(const IndexOptions &index_options) {
    if (initialized) return;
    initialized = true;

    auto &RS = RunSettings::get_instance();
    instance.m_index_settings_path = RunSettings::get_instance().get_logs_path() + instance.INDEX_SETTINGS_FILE;
    instance.file_setup(instance.m_index_settings_path, INDEX_SETTINGS_COL_STRS);

    uint segment_len = 0, pos_per_env = 0;
    SaxNumBitsT first_layer_num_bits = 0, num_bits_limit = 0;
    size_t leaf_capacity = 0;
    str brs_str = "", sps_str = "", min_num_bits_on_tie_str = "", method_type_str = "";

    if (index_options.index_params) {
        auto method_type = index_options.index_params->get_type();
        method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(method_type);

        if (method_type == ISAX_ENVELOPE) {
            auto *params = dynamic_cast<iSaxEnvelopeIndexParams *>(index_options.index_params.get());
            segment_len = params->segment_len;
            pos_per_env = params->pos_per_env;
            first_layer_num_bits = params->first_layer_num_bits;
            leaf_capacity = params->leaf_capacity;
            brs_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(params->breakpoint_strategy_type);

            auto split_strategy = params->split_strategy_type;
            sps_str = ISAX_SPLIT_STRATEGY_TO_STR.at(split_strategy);

            if (split_strategy == ENTROPY_MAXIMIZING) min_num_bits_on_tie_str = to_string(params->min_num_bits_on_tie);
            num_bits_limit = params->num_bits_limit;
        }
    }

    instance.m_columns = {
        {ISC::ID, to_string(instance.determine_index(instance.m_index_settings_path))},
        {ISC::DATASET_FILE, RS.m_dataset_props.file},
        {ISC::INDEX_FILE, RS.m_index_file},
        {ISC::FFTS_FILE, RS.m_ffts_file},
        {ISC::L_MIN, format_num_param(index_options.l_min)},
        {ISC::L_MAX, format_num_param(index_options.l_max)},
        {ISC::NORMALIZED, to_string(index_options.normalized)},
        {ISC::INDEX_TYPE, method_type_str},
        {ISC::SEGMENT_LENGTH, format_num_param(segment_len)},
        {ISC::POS_PER_ENV, format_num_param(pos_per_env)},
        {ISC::FIRST_LAYER_NUM_BITS, format_num_param(first_layer_num_bits)},
        {ISC::LEAF_CAPACITY, to_string(leaf_capacity)},
        {ISC::BREAKPOINT_STRATEGY, brs_str},
        {ISC::SPLIT_STRATEGY, sps_str},
        {ISC::MIN_NUM_BITS_ON_TIE, min_num_bits_on_tie_str},
        {ISC::NUM_BITS_LIMIT, format_num_param(num_bits_limit)},
    };
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_count_cols[col] = 0;
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_time_cols_duration[col] = 0;
}

void IndexLogger::increment_count_col(ISC col, uint amount) {
    assert(vec_contains(INDEX_COUNT_COLUMNS, col));
    instance.m_count_cols[col] += amount;
}

void IndexLogger::start_timer(ISC col) {
    assert(vec_contains(INDEX_TIME_COLUMNS, col));
    m_time_cols_start[col] = std::chrono::high_resolution_clock::now();
}

void IndexLogger::stop_timer(ISC col) {
    assert(vec_contains(INDEX_TIME_COLUMNS, col));
    auto end = std::chrono::high_resolution_clock::now();
    m_time_cols_duration[col] += std::chrono::duration<double>(end - m_time_cols_start[col]).count();
}

void IndexLogger::write_entry() {
    for (const auto &col : INDEX_COUNT_COLUMNS) instance.m_columns[col] = to_string(instance.m_count_cols[col]);
    for (const auto &col : INDEX_TIME_COLUMNS) instance.m_columns[col] = to_string(instance.m_time_cols_duration[col]);
    instance.write_row(m_index_settings_path, instance.m_columns, INDEX_SETTINGS_COL_ENUMS);
}

// QueryLogger
QueryLogger QueryLogger::instance = QueryLogger();
bool QueryLogger::initialized = false;
QueryLogger &QueryLogger::get_instance() {
    assert(initialized);
    return instance;
}

using QC = QueryColumn;
using QSC = QuerySettingsColumn;

void QueryLogger::initialize(const SearchOptions &search_options) {
    if (initialized) return;
    initialized = true;

    auto &RS = RunSettings::get_instance();

    str query_settings_path = RS.get_logs_path() + instance.QUERY_SETTINGS_FILE;

    // Write settings file
    instance.file_setup(query_settings_path, QUERY_SETTINGS_COL_STRS);

    instance.m_query_settings_id_str = to_string(instance.determine_index(query_settings_path));
    // Determine number of queries
    uint num_queries = 0;
    {
        std::ifstream query_stream_read(RS.get_query_path());
        str line;
        for (; !query_stream_read.eof(); ++num_queries) std::getline(query_stream_read, line);
        num_queries /= RS.m_dataset_props.num_channels;
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

    str early_abandon_str = "";
    DistanceType distance_type = search_options.distance_measure->get_type();
    if (distance_type == DistanceType::ED) {
        auto *ed = static_cast<EuclideanDistance *>(search_options.distance_measure.get());
        early_abandon_str = to_string(ed->uses_early_abandoning());
    }

    instance.write_row(
        query_settings_path,
        {
            {QSC::ID, instance.m_query_settings_id_str},
            {QSC::INDEX_FILE, RS.m_index_file},
            {QSC::DATASET_FILE, RS.m_dataset_props.file},
            {QSC::FFTS_FILE, RS.m_ffts_file},
            {QSC::QUERY_FILE, RS.m_query_properties.file},
            {QSC::NUM_QUERIES, to_string(num_queries)},
            {QSC::QUERY_TYPE, SEARCH_TYPE_TO_STR.at(search_type)},
            {QSC::R_RANGE_R, format_num_param(r_range_r)},
            {QSC::KNN_K, format_num_param(knn_k)},
            {QSC::EXACT, to_string(search_options.exact)},
            {QSC::NORMALIZED, to_string(search_options.normalized)},
            {QSC::SEARCH_METHOD, SEARCH_METHOD_TYPE_TO_STR.at(search_options.search_method_type)},
            {QSC::DISTANCE_MEASURE, DISTANCE_TYPE_TO_STR.at(search_options.distance_measure->get_type())},
            {QSC::EARLY_ABANDONING, early_abandon_str},
        },
        QUERY_SETTINGS_COL_ENUMS);

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

void QueryLogger::increment_count_col(QC col) {
    assert(vec_contains(QUERY_COUNT_COLUMNS, col));
    ++instance.m_count_cols[col];
}

void QueryLogger::start_timer(QC col) {
    assert(vec_contains(QUERY_TIME_COLUMNS, col));
    instance.m_time_cols_start[col] = std::chrono::high_resolution_clock::now();
}

void QueryLogger::stop_timer(QC col) {
    assert(vec_contains(QUERY_TIME_COLUMNS, col));
    auto end = std::chrono::high_resolution_clock::now();
    instance.m_time_cols_duration[col] += std::chrono::duration<double>(end - instance.m_time_cols_start[col]).count();
}

void QueryLogger::log_query(const vec<vec<float>> &query) {
    size_t query_len = 0;
    vec<str> included;

    for (const auto &channel : query) {
        included.push_back(channel.empty() ? "0" : "1");
        query_len = std::max(query_len, channel.size());
    }
    instance.m_settable_cols[QC::QUERY_LENGTH] = to_string(query_len);
    instance.m_collection_cols[QC::QUERY_CHANNELS] = included;
}

void QueryLogger::log_results(const vec<SearchResult> &results) {
    auto &RS = RunSettings::get_instance();
    size_t series_size = RS.m_dataset_props.series_len * RS.m_dataset_props.num_channels;
    for (auto result : results) {
        auto [ts_index, ts_position] = result.subs_pos;
        instance.m_collection_cols[QC::RESULT_SET_TS_INDICES].push_back(to_string(ts_index));
        instance.m_collection_cols[QC::RESULT_SET_TS_POSITIONS].push_back(to_string(ts_position));
        instance.m_collection_cols[QC::RESULT_SET_DISTANCES].push_back(to_string(result.distance));
    }
}

str QueryLogger::get_collection_str(QC col) {
    assert(vec_contains(QUERY_COLLECTION_COLUMNS, col));
    str result;
    for (uint i = 0; i < instance.m_collection_cols[col].size(); ++i) {
        result += instance.m_collection_cols[col][i];
        if (i < instance.m_collection_cols[col].size() - 1) result += ITEM_SEP;
    }
    return result;
}

void QueryLogger::write_entry() {
    str run_log_path = fs::path(RunSettings::get_instance().get_logs_path()) / instance.RUN_LOG_FILE;
    umap<QC, str> columns({
        {QC::ID, to_string(instance.determine_index(run_log_path))},
        {QC::SETTINGS_ID, m_query_settings_id_str},
    });

    for (const auto &col : QUERY_NUMBER_COLUMNS) columns[col] = m_settable_cols[col];
    for (const auto &col : QUERY_COUNT_COLUMNS) columns[col] = to_string(m_count_cols[col]);
    for (const auto &col : QUERY_TIME_COLUMNS) columns[col] = to_string(m_time_cols_duration[col]);
    for (const auto &col : QUERY_COLLECTION_COLUMNS) columns[col] = get_collection_str(col);

    write_row(run_log_path, columns, QUERY_COL_ENUMS);
}

// QueryStatsLogger
void QueryStatsLogger::write_entry(uint query_id, const vec<vec<float>> &query, QueryStats stats, bool normalized,
                                   float noise) {
#ifndef DISABLE_LOGGING
    QueryStatsLogger instance;
    auto &RS = RunSettings::get_instance();

    str dataset_file = RS.m_dataset_props.file;
    str query_file = RS.m_query_properties.file;

    size_t query_len = 0;
    str query_channels_str = "";
    for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
        query_channels_str += query[c].empty() ? "0" : "1";
        if (c < query.size() - 1) query_channels_str += instance.ITEM_SEP;
        query_len = std::max(query_len, query[c].size());
    }
    str query_len_str = to_string(query_len);

    str query_stats_path = fs::path(RS.get_logs_path()) / instance.QUERY_STATS_FILE;
    instance.file_setup(query_stats_path, QUERY_STATS_COL_STRS);
    instance.write_row(query_stats_path,
                       {
                           {QSTC::ID, to_string(query_id)},
                           {QSTC::DATASET_FILE, dataset_file},
                           {QSTC::QUERY_FILE, query_file},
                           {QSTC::QUERY_LENGTH, query_len_str},
                           {QSTC::QUERY_CHANNELS, query_channels_str},
                           {QSTC::MIN_DIST, to_string(stats.min_dist)},
                           {QSTC::MAX_DIST, to_string(stats.max_dist)},
                           {QSTC::MEAN_DIST, to_string(stats.mean_dist)},
                           {QSTC::DIST_STD_DEV, to_string(stats.dist_std_dev)},
                           {QSTC::RC_USING_MAX, to_string(stats.rc_using_max)},
                           {QSTC::RC_USING_MEAN, to_string(stats.rc_using_mean)},
                           {QSTC::NORMALIZED, to_string(normalized)},
                           {QSTC::QUERY_NOISE, to_string(noise)},
                       },
                       QUERY_STATS_COL_ENUMS);
#endif
}
