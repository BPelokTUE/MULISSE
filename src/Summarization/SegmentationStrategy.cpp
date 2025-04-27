#include "Summarization/SegmentationStrategy.hpp"

// UniformSegmentationStrategy

UniformSegmentationStrategy::UniformSegmentationStrategy(uint l_max, SaxSegIndT num_segments)
    : m_segment_len(l_max / num_segments), m_l_max(l_max) {}

SaxSegIndT UniformSegmentationStrategy::get_num_segments(uint subs_len) const {
    assert(subs_len <= m_l_max);
    return static_cast<SaxSegIndT>(subs_len / m_segment_len);
}

uint UniformSegmentationStrategy::get_segment_len(SaxSegIndT segment_ind) const { return m_segment_len; }

SegmentationStrategyType UniformSegmentationStrategy::get_type() const { return UNIFORM; }

// AdaptiveSegmentationStrategy

AdaptiveSegmentationStrategy::AdaptiveSegmentationStrategy(uint l_min, uint l_max, uint series_len,
                                                           SaxSegIndT num_segments)
    : m_l_max(l_max) {
    auto [presences, presences_sum] = calculate_presences(l_min, l_max, series_len);

    SaxSegIndT segments_remaining = num_segments;
    size_t segment_presence = presences_sum / segments_remaining;

    m_segment_lens.reserve(num_segments);
    size_t presences_acc = 0;
    uint l_start = 1;
    for (uint l = 1; l <= l_max; ++l) {
        presences_acc += presences[l];
        presences_sum -= presences[l];
        if (presences_acc >= segment_presence) {
            m_segment_lens.push_back(l - l_start + 1);
            m_segment_ends.push_back(l);
            presences_acc = 0;
            l_start = l + 1;

            --segments_remaining;
            segment_presence = presences_sum / segments_remaining;
            if (segments_remaining == 1) break;
        }
    }
    m_segment_ends.push_back(l_max);
    m_segment_lens.push_back(l_max - l_start + 1);
}

std::pair<vec<size_t>, size_t> AdaptiveSegmentationStrategy::calculate_presences(uint l_min, uint l_max,
                                                                                 uint series_len) const {
    assert(l_min > 0 && l_max > l_min);

    vec<size_t> presences(l_max + 2, 0);
    size_t presences_sum = 0;
    for (uint l = l_max; l >= l_min; --l) {
        presences[l] = presences[l + 1] + (series_len - l + 1);
        presences_sum += presences[l];
    }
    for (uint l = l_min - 1; l > 0; --l) {
        presences[l] = presences[l + 1];
        presences_sum += presences[l];
    }
    return {presences, presences_sum};
}

SaxSegIndT AdaptiveSegmentationStrategy::get_num_segments(uint subs_len) const {
    assert(subs_len <= m_l_max);
    SaxSegIndT end_ind = static_cast<SaxSegIndT>(
        std::lower_bound(m_segment_ends.begin(), m_segment_ends.end(), subs_len) - m_segment_ends.begin());
    return subs_len == m_segment_ends[end_ind] ? end_ind + 1 : end_ind;
}

uint AdaptiveSegmentationStrategy::get_segment_len(SaxSegIndT segment_ind) const { return m_segment_lens[segment_ind]; }

SegmentationStrategyType AdaptiveSegmentationStrategy::get_type() const { return ADAPTIVE; }
