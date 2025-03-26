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
#endif  // DISABLE_LOGGING
    return index;
}

void Logger::file_setup(const str &file_path, const vec<str> &header) {
#ifndef DISABLE_LOGGING
    // If the directory does not exist, create it
    std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());

    if (std::filesystem::exists(file_path)) {
        std::ifstream file(file_path);
        if (file.peek() != std::ifstream::traits_type::eof()) {
            return;
        }
    }

    std::ofstream ofs(file_path);
    for (uint i = 0; i < header.size(); ++i) {
        ofs << header[i];
        if (i < header.size() - 1) ofs << COL_SEP;
    }
#endif  // DISABLE_LOGGING
}

// DatasetLogger
RandomWalkLogAttributes::RandomWalkLogAttributes(float noise, int seed) : noise(noise), seed(seed) {}

DatasetType RandomWalkLogAttributes::get_type() { return RANDOM_WALK; }

CsvDatasetLogAttributes::CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint l_min,
                                                 uint l_max, int seed)
    : source_csvs(source_csvs), series_generated(series_generated), l_min(l_min), l_max(l_max), seed(seed) {}

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

    str sd_str = "", source_csv_str = "", l_min_str = "", l_max_str = "", seed_str = "";
    switch (attributes->get_type()) {
        case RANDOM_WALK: {
            auto *rw_attributes = static_cast<RandomWalkLogAttributes *>(attributes.get());
            sd_str = to_string(rw_attributes->noise);
            seed_str = to_string(rw_attributes->seed);
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
            l_min_str = to_string(csv_attributes->l_min);
            l_max_str = to_string(csv_attributes->l_max);
            seed_str = to_string(csv_attributes->seed);
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
                           {DSC::L_MIN, l_min_str},
                           {DSC::L_MAX, l_max_str},
                           {DSC::SEED, seed_str},
                       },
                       DATASET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

// QuerySetLogger
using QSC = QuerySetSettingsColumn;

void QuerySetLogger::write_entry(QuerySetOptions &opts) {
#ifndef DISABLE_LOGGING
    QuerySetLogger instance;

    auto &RS = RunSettings::get_instance();

    str query_settings_path = fs::path(RS.get_logs_path()) / instance.QUERY_SET_SETTINGS_FILE;
    instance.file_setup(query_settings_path, QUERY_SET_SETTINGS_COL_STRS);

    instance.write_row(query_settings_path,
                       {
                           {QSC::ID, to_string(instance.determine_index(query_settings_path))},
                           {QSC::DATASET_FILE, RS.get_dataset_props().file},
                           {QSC::QUERY_FILE, RS.get_query_props().file},
                           {QSC::NUM_QUERIES, to_string(opts.num_queries)},
                           {QSC::L_MIN, format_num_param(opts.l_min)},
                           {QSC::L_MAX, format_num_param(opts.l_max)},
                           {QSC::EXACT_LENGTHS, instance.get_num_vec_str(opts.exact_lengths)},
                           {QSC::USED_CHANNELS, format_num_param(opts.used_channels)},
                           {QSC::CHANNEL_MASK, instance.get_num_vec_str(opts.channel_mask)},
                           {QSC::NOISE, to_string(opts.noise)},
                           {QSC::SEED, to_string(opts.seed)},
                       },
                       QUERY_SET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

// IndexLogger
IndexLogger IndexLogger::instance = IndexLogger();
bool IndexLogger::initialized = false;
IndexLogger &IndexLogger::get_instance() { return instance; }

using ISC = IndexSettingsColumn;

void IndexLogger::initialize(const IndexOptions &index_options) {
    if (initialized) return;
    initialized = true;

    auto &RS = RunSettings::get_instance();
    instance.m_index_settings_path =
        fs::path(RunSettings::get_instance().get_logs_path()) / instance.INDEX_SETTINGS_FILE;
    instance.file_setup(instance.m_index_settings_path, INDEX_SETTINGS_COL_STRS);

    uint segment_len = 0, pos_per_env = 0;
    SaxNumBitsT first_layer_num_bits = 0, num_bits_limit = 0;
    size_t leaf_capacity = 0;
    str brs_str = "", sps_str = "", min_num_bits_on_tie_str = "", method_type_str = "";

    if (index_options.index_params) {
        auto method_type = index_options.index_params->get_type();
        method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(method_type);

        if (method_type == ISAX || method_type == ISAX_ENVELOPE) {
            auto *params = dynamic_cast<iSaxIndexParams *>(index_options.index_params.get());
            segment_len = params->segment_len;
            first_layer_num_bits = params->first_layer_num_bits;
            leaf_capacity = params->leaf_capacity;
            brs_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(params->breakpoint_strategy_type);

            auto split_strategy = params->split_strategy_type;
            sps_str = ISAX_SPLIT_STRATEGY_TO_STR.at(split_strategy);

            if (split_strategy == ENTROPY_MAXIMIZING) min_num_bits_on_tie_str = to_string(params->min_num_bits_on_tie);
            num_bits_limit = params->num_bits_limit;

            if (method_type == ISAX_ENVELOPE) {
                auto *env_params = dynamic_cast<iSaxEnvelopeIndexParams *>(params);
                pos_per_env = env_params->pos_per_env;
            }
        } else if (method_type == ENVELOPE) {
            auto *params = dynamic_cast<EnvelopeIndexParams *>(index_options.index_params.get());
            segment_len = params->segment_len;
            pos_per_env = params->pos_per_env;
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
        {ISC::LEAF_CAPACITY, format_num_param(leaf_capacity)},
        {ISC::BREAKPOINT_STRATEGY, brs_str},
        {ISC::SPLIT_STRATEGY, sps_str},
        {ISC::MIN_NUM_BITS_ON_TIE, min_num_bits_on_tie_str},
        {ISC::NUM_BITS_LIMIT, format_num_param(num_bits_limit)},
        {ISC::ADAPT_TO_DATASET, to_string(index_options.adapt)},
        {ISC::INSERTER_TYPE, ENTRY_INSERTER_TYPE_TO_STR.at(index_options.inserter_type)},
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
QueryLogger &QueryLogger::get_instance() { return instance; }

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
        search_settings_path,
        {
            {SSC::ID, instance.m_search_settings_id_str},
            {SSC::INDEX_FILE, RS.m_index_file},
            {SSC::DATASET_FILE, RS.m_dataset_props.file},
            {SSC::FFTS_FILE, RS.m_ffts_file},
            {SSC::QUERY_FILE, RS.m_query_properties.file},
            {SSC::NUM_QUERIES, to_string(num_queries)},
            {SSC::QUERY_TYPE, SEARCH_TYPE_TO_STR.at(search_type)},
            {SSC::R_RANGE_R, format_num_param(r_range_r)},
            {SSC::KNN_K, format_num_param(knn_k)},
            {SSC::EXACT, to_string(search_options.exact)},
            {SSC::NORMALIZED, to_string(search_options.normalized)},
            {SSC::SEARCH_METHOD, SEARCH_METHOD_TYPE_TO_STR.at(search_options.search_method_type)},
            {SSC::DISTANCE_MEASURE, DISTANCE_TYPE_TO_STR.at(search_options.distance_measure->get_type())},
            {SSC::EARLY_ABANDONING, early_abandon_str},
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

void QueryLogger::increment_count_col(QC col, uint amount) {
    assert(vec_contains(QUERY_COUNT_COLUMNS, col));
    instance.m_count_cols[col] += amount;
}

void QueryLogger::increment_num_points_in_examined_entries(uint64_t amount) {
    num_points_in_examined_entries += amount;
}

void QueryLogger::increment_num_points_examined(uint64_t amount) { num_points_examined += amount; }

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
        auto [ts_index, ts_position, ts_length] = result.subs_info;
        instance.m_collection_cols[QC::RESULT_SET_TS_INDICES].push_back(to_string(ts_index));
        instance.m_collection_cols[QC::RESULT_SET_TS_POSITIONS].push_back(to_string(ts_position));
        instance.m_collection_cols[QC::RESULT_SET_DISTANCES].push_back(to_string(std::sqrt(result.distance)));
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
        {QC::SETTINGS_ID, m_search_settings_id_str},
    });

    for (const auto &col : QUERY_NUMBER_COLUMNS) columns[col] = m_settable_cols[col];
    for (const auto &col : QUERY_COUNT_COLUMNS) columns[col] = to_string(m_count_cols[col]);
    for (const auto &col : QUERY_TIME_COLUMNS) columns[col] = to_string(m_time_cols_duration[col]);
    for (const auto &col : QUERY_COLLECTION_COLUMNS) columns[col] = get_collection_str(col);

    bool abandoning_used = num_points_examined < num_points_in_examined_entries;
    columns[QC::ABANDONING_RATE] = to_string(
        abandoning_used ? 1.0 - static_cast<double>(num_points_examined) / num_points_in_examined_entries : 0.0);

    write_row(run_log_path, columns, QUERY_COL_ENUMS);
}

// Stats

AttributeStats::AttributeStats() {
    min = INF;
    max = sum = sum_sq = 0;
}

void AttributeStats::update(float value) {
    min = std::min(min, value);
    max = std::max(max, value);
    sum += value;
    sum_sq += value * value;
}

void AttributeStats::update(float value, size_t count) {
    min = std::min(min, value);
    max = std::max(max, value);
    sum += value * count;
    sum_sq += value * value * count;
}

void AttributeStats::calculate(uint count) {
    auto mu_and_sigma = calculate_mu_and_sigma(sum, sum_sq, count);
    mean = mu_and_sigma.first;
    st_dev = mu_and_sigma.second;
}

void QueryStats::calculate() {
    dist_stats.calculate(subs_count);
    rc_using_max = (dist_stats.max - dist_stats.min) / dist_stats.min;
    rc_using_mean = dist_stats.mean / dist_stats.min;
}

void IndexStats::update_leaf_stats(float fill, float height) {
    leaf_size_stats.update(fill);
    leaf_height_stats.update(height);
    ++leaf_count;
}

void IndexStats::update_seg_stats(float lower, float upper, size_t count) {
    if (count == 0) return;

    bool lower_inf = lower == -INF, upper_inf = upper == INF;
    num_inf_lower += lower_inf;
    num_inf_upper += upper_inf;
    if (lower_inf || upper_inf) return;

    seg_lower_stats.update(lower, count);
    seg_upper_stats.update(upper, count);
    seg_range_stats.update(upper - lower, count);
    seg_count += count;
}

void IndexStats::calculate() {
    vec<AttributeStats *> leaf_type_stats = {&leaf_size_stats, &leaf_height_stats},
                          seg_type_stats = {&seg_range_stats, &seg_lower_stats, &seg_upper_stats};
    for (AttributeStats *leaf_stats : leaf_type_stats) leaf_stats->calculate(leaf_count);
    for (AttributeStats *seg_stats : seg_type_stats) seg_stats->calculate(seg_count);
}

// QueryStatsLogger

// clang-format off
#define ADD_STATS_TO_ROW(ENUM, SUFFIX, stats)       \
    {ENUM::MIN_##SUFFIX, to_string(stats.min)},   \
    {ENUM::MAX_##SUFFIX, to_string(stats.max)},   \
    {ENUM::MEAN_##SUFFIX, to_string(stats.mean)}, \
    {ENUM::STD_##SUFFIX, to_string(stats.st_dev)}
// clang-format on

using QSTC = QueryStatsColumn;

void QueryStatsLogger::write_entry(uint query_id, const vec<vec<float>> &query, QueryStats stats, bool normalized) {
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
                           ADD_STATS_TO_ROW(QSTC, DIST, stats.dist_stats),
                           {QSTC::RC_USING_MAX, to_string(stats.rc_using_max)},
                           {QSTC::RC_USING_MEAN, to_string(stats.rc_using_mean)},
                           {QSTC::NORMALIZED, to_string(normalized)},
                       },
                       QUERY_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

// IndexStatsLogger

using ISTC = IndexStatsColumn;

void IndexStatsLogger::write_entry(const IndexStats &stats) {
#ifndef DISABLE_LOGGING
    IndexStatsLogger instance;
    auto &RS = RunSettings::get_instance();

    str index_file = RS.m_index_file;
    str index_stats_path = fs::path(RS.get_logs_path()) / instance.INDEX_STATS_FILE;
    instance.file_setup(index_stats_path, INDEX_STATS_COL_STRS);
    instance.write_row(index_stats_path,
                       {
                           {ISTC::INDEX_FILE, index_file},
                           ADD_STATS_TO_ROW(ISTC, LEAF_SIZE, stats.leaf_size_stats),
                           ADD_STATS_TO_ROW(ISTC, LEAF_HEIGHT, stats.leaf_height_stats),
                           ADD_STATS_TO_ROW(ISTC, SEG_RANGE, stats.seg_range_stats),
                           ADD_STATS_TO_ROW(ISTC, SEG_LOWER, stats.seg_lower_stats),
                           ADD_STATS_TO_ROW(ISTC, SEG_UPPER, stats.seg_upper_stats),
                           {ISTC::NUM_INF_LOWER, to_string(stats.num_inf_lower)},
                           {ISTC::NUM_INF_UPPER, to_string(stats.num_inf_upper)},
                       },
                       INDEX_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}
