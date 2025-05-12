#include "Index/Segmentation/SegmentationStrategy/AdaptiveSegmentationStrategy.hpp"

#include "Index/Segmentation/Presence.hpp"

AdaptiveSegmentationStrategy::AdaptiveSegmentationStrategy(uint l_min, uint l_max, uint series_len,
                                                           SaxSegIndT num_segments, uint pos_per_env)
    : m_l_max(l_max) {
    PresenceArray presence_array(l_min, l_max, series_len, pos_per_env);
    const auto &presences = presence_array.get_presences();
    auto presence_sum = presence_array.get_presence_sum();

    SaxSegIndT segments_remaining = num_segments;
    size_t segment_presence = presence_sum / segments_remaining;

    m_segment_lens.reserve(num_segments);
    size_t presences_acc = 0;
    uint l_start = 1;
    for (uint l = 1; l <= l_max; ++l) {
        presences_acc += presences[l];
        presence_sum -= presences[l];
        if (presences_acc >= segment_presence) {
            m_segment_lens.push_back(l - l_start + 1);
            m_segment_ends.push_back(l);
            presences_acc = 0;
            l_start = l + 1;

            --segments_remaining;
            segment_presence = presence_sum / segments_remaining;
            if (segments_remaining == 1) break;
        }
    }
    m_segment_ends.push_back(l_max);
    m_segment_lens.push_back(l_max - l_start + 1);
}

SaxSegIndT AdaptiveSegmentationStrategy::get_num_segments(uint subs_len) const {
    assert(subs_len <= m_l_max);
    assert(subs_len >= m_segment_ends[0]);
    SaxSegIndT end_ind = static_cast<SaxSegIndT>(
        std::lower_bound(m_segment_ends.begin(), m_segment_ends.end(), subs_len) - m_segment_ends.begin());
    return subs_len == m_segment_ends[end_ind] ? end_ind + 1 : end_ind;
}

uint AdaptiveSegmentationStrategy::get_segment_len(SaxSegIndT segment_ind) const { return m_segment_lens[segment_ind]; }

SegmentationStrategyType AdaptiveSegmentationStrategy::get_type() const { return ADAPTIVE; }
