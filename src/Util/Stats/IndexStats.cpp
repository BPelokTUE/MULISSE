#include "Util/Stats/IndexStats.hpp"

#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

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

    Real mid = (lower + upper) / 2;
    m_seg_lower_stats.update(lower, count);
    m_seg_upper_stats.update(upper, count);
    m_seg_mid_stats.update(mid, count);
    m_seg_range_stats.update(upper - lower, count);
    m_seg_count += count;

    if (m_separate_segment_stats && channel_ind < m_seg_count_list.size() &&
        seg_ind < m_seg_count_list[channel_ind].size()) {
        m_seg_lower_list_stats[channel_ind][seg_ind].update(lower, count);
        m_seg_upper_list_stats[channel_ind][seg_ind].update(upper, count);
        m_seg_mid_list_stats[channel_ind][seg_ind].update(mid, count);
        m_seg_range_list_stats[channel_ind][seg_ind].update(upper - lower, count);
        m_seg_count_list[channel_ind][seg_ind] += count;
    }
}

void IndexStats::calculate() {
    vec<AttributeStats *> leaf_type_stats = {&m_leaf_size_stats, &m_leaf_height_stats},
                          seg_type_stats = {&m_seg_range_stats, &m_seg_lower_stats, &m_seg_upper_stats,
                                            &m_seg_mid_stats};
    for (AttributeStats *leaf_stats : leaf_type_stats) leaf_stats->calculate(U(m_leaf_count));
    for (AttributeStats *seg_stats : seg_type_stats) seg_stats->calculate(U(m_seg_count));

    vec<vec<vec<AttributeStats>> *> seg_list_type_stats = {&m_seg_range_list_stats, &m_seg_lower_list_stats,
                                                           &m_seg_upper_list_stats, &m_seg_mid_list_stats};
    if (m_separate_segment_stats) {
        for (auto seg_list_stats : seg_list_type_stats)
            for (MtsNumChannelsT c = 0; c < seg_list_stats->size(); ++c)
                for (SaxSegIndT s = 0; s < (*seg_list_stats)[c].size(); ++s)
                    (*seg_list_stats)[c][s].calculate(U(m_seg_count_list[c][s]));
    }
}
