#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToProportionalNumSegments.hpp"

#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

ScoreToProportionalNumSegments::ScoreToProportionalNumSegments(uint num_segments_total,
                                                               const NumSegToSegmentationStrategy &strategy_factory)
    : m_num_segments_total(num_segments_total), m_strategy_factory(std::move(strategy_factory)) {}

ScoreToProportionalNumSegments::ScoreToProportionalNumSegments(SaxSegIndT num_channels_avg,
                                                               MtsNumChannelsT num_channels,
                                                               const NumSegToSegmentationStrategy &strategy_factory)
    : m_num_segments_total(U(num_channels_avg * num_channels)), m_strategy_factory(std::move(strategy_factory)) {}

vec<sptr<ISegmentationStrategy>> ScoreToProportionalNumSegments::get_segmentation_strategy(
    const vec<Real> &scores) const {
    Real score_total = 0.0;
    for (Real score : scores) score_total += score;

    uint num_segments_remaining = m_num_segments_total;
    vec<sptr<ISegmentationStrategy>> segmentation_strategies;
    segmentation_strategies.reserve(scores.size());

    for (Real score : scores) {
        SaxSegIndT num_segments = static_cast<SaxSegIndT>(std::round(score / score_total * R(num_segments_remaining)));
        segmentation_strategies.push_back(m_strategy_factory(num_segments));
        num_segments_remaining -= num_segments;
        score_total -= score;
    }
    return segmentation_strategies;
}
