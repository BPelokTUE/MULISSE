#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"

UniformSegmentationStrategy::UniformSegmentationStrategy(uint l_max, SaxSegIndT num_segments)
    : m_segment_len(l_max / num_segments), m_l_max(l_max) {
    assert(l_max >= num_segments);
}

SaxSegIndT UniformSegmentationStrategy::get_num_segments(uint subs_len) const {
    assert(subs_len <= m_l_max);
    assert(subs_len >= m_segment_len);
    return static_cast<SaxSegIndT>(subs_len / m_segment_len);
}

uint UniformSegmentationStrategy::get_segment_len(SaxSegIndT segment_ind) const { return m_segment_len; }

SegmentationStrategyType UniformSegmentationStrategy::get_type() const { return UNIFORM; }
