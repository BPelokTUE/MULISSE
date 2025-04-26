#include "Summarization/SegmentationStrategy.hpp"

UniformSegmentationStrategy::UniformSegmentationStrategy(uint l_max, SaxSegIndT num_segments)
    : m_segment_len(l_max / num_segments) {}

SaxSegIndT UniformSegmentationStrategy::get_num_segments(uint ts_size) const {
    return static_cast<SaxSegIndT>(ts_size / m_segment_len);
}

uint UniformSegmentationStrategy::get_segment_len(SaxSegIndT segment_ind) const { return m_segment_len; }

SegmentationStrategyType UniformSegmentationStrategy::get_type() const { return UNIFORM; }
