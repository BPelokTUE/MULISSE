#ifndef INDEX_STATS_INDEXSTATS_HPP
#define INDEX_STATS_INDEXSTATS_HPP

#include "Util/Stats/AttributesStats.hpp"
#include "Util/Types/Containers.hpp"

struct IndexStats {
    bool m_separate_segment_stats = false;

    AttributeStats m_leaf_size_stats;
    AttributeStats m_leaf_height_stats;
    AttributeStats m_seg_range_stats;
    AttributeStats m_seg_lower_stats;
    AttributeStats m_seg_upper_stats;
    AttributeStats m_seg_mid_stats;
    vec<vec<AttributeStats>> m_seg_range_list_stats;
    vec<vec<AttributeStats>> m_seg_lower_list_stats;
    vec<vec<AttributeStats>> m_seg_upper_list_stats;
    vec<vec<AttributeStats>> m_seg_mid_list_stats;

    size_t m_leaf_count = 0, m_seg_count = 0;
    vec<vec<size_t>> m_seg_count_list;
    size_t m_num_inf_lower = 0, m_num_inf_upper = 0;

    IndexStats() = default;

    IndexStats(MtsNumChannelsT num_channels, SaxSegIndT num_segments_per_channel, bool separate_segment_stats = false);

    void update_leaf_stats(size_t num_entries, size_t height);

    void update_seg_stats(Real lower, Real upper, MtsNumChannelsT channel_ind, SaxSegIndT seg_ind, size_t count = 1);

    void calculate();
};

#endif  // INDEX_STATS_INDEXSTATS_HPP
