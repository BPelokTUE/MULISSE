#include "Util/Logging/IndexStatsLogger.hpp"

#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

// clang-format off
#define ADD_STATS_LIST_TO_ROW(ENUM, SUFFIX, stats_list)       \
    {ENUM::MIN_##SUFFIX, instance.get_collection_str(stats_list.m_mins)},   \
    {ENUM::MAX_##SUFFIX, instance.get_collection_str(stats_list.m_maxs)},   \
    {ENUM::MEAN_##SUFFIX, instance.get_collection_str(stats_list.m_means)}, \
    {ENUM::STD_##SUFFIX, instance.get_collection_str(stats_list.m_st_devs)}
// clang-format on

struct FlatStatsList {
    vec<Real> m_mins, m_maxs, m_means, m_st_devs;

    FlatStatsList(const vec<vec<AttributeStats>> &stats_list) {
        if (stats_list.empty() || stats_list[0].empty()) return;

        size_t flat_stats_list_size = stats_list.size() * stats_list[0].size();

        m_mins.resize(flat_stats_list_size);
        m_maxs.resize(flat_stats_list_size);
        m_means.resize(flat_stats_list_size);
        m_st_devs.resize(flat_stats_list_size);

        for (MtsNumChannelsT c = 0; c < stats_list.size(); ++c) {
            for (SaxSegIndT s = 0; s < stats_list[c].size(); ++s) {
                size_t index = c * stats_list[0].size() + s;
                m_mins[index] = stats_list[c][s].m_min;
                m_maxs[index] = stats_list[c][s].m_max;
                m_means[index] = stats_list[c][s].m_mean;
                m_st_devs[index] = stats_list[c][s].m_st_dev;
            }
        }
    }
};

using ISTC = IndexStatsColumn;

IndexStats::IndexStats(MtsNumChannelsT num_channels, SaxSegIndT num_segments_per_channel, bool separate_segment_stats)
    : m_separate_segment_stats(separate_segment_stats),
      m_seg_count_list(num_channels, vec<size_t>(num_segments_per_channel, 0)),
      m_seg_lower_list_stats(num_channels, vec<AttributeStats>(num_segments_per_channel)),
      m_seg_upper_list_stats(num_channels, vec<AttributeStats>(num_segments_per_channel)),
      m_seg_range_list_stats(num_channels, vec<AttributeStats>(num_segments_per_channel)) {}

void IndexStats::update_leaf_stats(size_t num_entries, size_t height) {
    m_leaf_size_stats.update(R(num_entries));
    m_leaf_height_stats.update(R(height));
    ++m_leaf_count;
}

void IndexStats::update_seg_stats(Real lower, Real upper, MtsNumChannelsT channel_ind, SaxSegIndT seg_ind,
                                  size_t count) {
    if (count == 0) return;

    bool lower_inf = lower == -INF, upper_inf = upper == INF;
    m_num_inf_lower += lower_inf;
    m_num_inf_upper += upper_inf;
    if (lower_inf || upper_inf) return;

    m_seg_lower_stats.update(lower, count);
    m_seg_upper_stats.update(upper, count);
    m_seg_range_stats.update(upper - lower, count);
    m_seg_count += count;

    if (m_separate_segment_stats && channel_ind < m_seg_count_list.size() &&
        seg_ind < m_seg_count_list[channel_ind].size()) {
        m_seg_lower_list_stats[channel_ind][seg_ind].update(lower, count);
        m_seg_upper_list_stats[channel_ind][seg_ind].update(upper, count);
        m_seg_range_list_stats[channel_ind][seg_ind].update(upper - lower, count);
        m_seg_count_list[channel_ind][seg_ind] += count;
    }
}

void IndexStats::calculate() {
    vec<AttributeStats *> leaf_type_stats = {&m_leaf_size_stats, &m_leaf_height_stats},
                          seg_type_stats = {&m_seg_range_stats, &m_seg_lower_stats, &m_seg_upper_stats};
    for (AttributeStats *leaf_stats : leaf_type_stats) leaf_stats->calculate(U(m_leaf_count));
    for (AttributeStats *seg_stats : seg_type_stats) seg_stats->calculate(U(m_seg_count));

    vec<vec<vec<AttributeStats>> *> seg_list_type_stats = {&m_seg_range_list_stats, &m_seg_lower_list_stats,
                                                           &m_seg_upper_list_stats};
    if (m_separate_segment_stats) {
        for (auto seg_list_stats : seg_list_type_stats)
            for (MtsNumChannelsT c = 0; c < seg_list_stats->size(); ++c)
                for (SaxSegIndT s = 0; s < (*seg_list_stats)[c].size(); ++s)
                    (*seg_list_stats)[c][s].calculate(U(m_seg_count_list[c][s]));
    }
}

const str IndexStatsLogger::INDEX_STATS_FILE = "index_stats.csv";

void IndexStatsLogger::write_entry(const IndexStats &stats, uint length_group_id, uint sub_index_id,
                                   bool separate_segment_stats) {
#ifndef DISABLE_LOGGING
    IndexStatsLogger instance;
    auto &RS = RunSettings::get_instance();

    str index_file = RS.m_index_file;
    str index_stats_path = fs::path(RS.get_logs_path()) / IndexStatsLogger::INDEX_STATS_FILE;

    instance.file_setup(index_stats_path, INDEX_STATS_COL_STRS);
    umap<IndexStatsColumn, str> enum_map = {
        {ISTC::INDEX_FILE, index_file},
        {ISTC::LENGTH_GROUP_ID, to_string(length_group_id)},
        {ISTC::SUB_INDEX_ID, to_string(sub_index_id)},
        ADD_STATS_TO_ROW(ISTC, LEAF_SIZE, stats.m_leaf_size_stats),
        ADD_STATS_TO_ROW(ISTC, LEAF_HEIGHT, stats.m_leaf_height_stats),
        ADD_STATS_TO_ROW(ISTC, SEG_RANGE, stats.m_seg_range_stats),
        ADD_STATS_TO_ROW(ISTC, SEG_LOWER, stats.m_seg_lower_stats),
        ADD_STATS_TO_ROW(ISTC, SEG_UPPER, stats.m_seg_upper_stats),
        {ISTC::NUM_INF_LOWER, to_string(stats.m_num_inf_lower)},
        {ISTC::NUM_INF_UPPER, to_string(stats.m_num_inf_upper)},
    };
    if (stats.m_separate_segment_stats) {
        vec<size_t> flat_seg_count_list(stats.m_seg_count_list.size() * stats.m_seg_count_list[0].size());
        for (MtsNumChannelsT c = 0; c < stats.m_seg_count_list.size(); ++c)
            for (SaxSegIndT s = 0; s < stats.m_seg_count_list[c].size(); ++s)
                flat_seg_count_list[c * stats.m_seg_count_list[0].size() + s] = stats.m_seg_count_list[c][s];

        umap<IndexStatsColumn, str> seg_list_map = {
            ADD_STATS_LIST_TO_ROW(ISTC, SEG_LOWER_LIST, FlatStatsList(stats.m_seg_lower_list_stats)),
            ADD_STATS_LIST_TO_ROW(ISTC, SEG_UPPER_LIST, FlatStatsList(stats.m_seg_upper_list_stats)),
            ADD_STATS_LIST_TO_ROW(ISTC, SEG_RANGE_LIST, FlatStatsList(stats.m_seg_range_list_stats)),
            {ISTC::SEGMENT_COUNT_LIST, instance.get_collection_str(flat_seg_count_list)},
        };
        enum_map.insert(seg_list_map.begin(), seg_list_map.end());
    }

    instance.write_row(index_stats_path, enum_map, INDEX_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}
