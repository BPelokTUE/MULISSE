#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreToProportionalNumSegments.hpp"

#include <cmath>

#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

ScoreToProportionalNumSegments::ScoreToProportionalNumSegments(
    uint num_segments_total, const std::function<sptr<ISegmentationStrategy>(SaxSegIndT)> &strategy_factory,
    Real score_exponent)
    : m_num_segments_total(num_segments_total),
      m_strategy_factory(std::move(strategy_factory)),
      m_score_exponent(score_exponent) {}

ScoreToProportionalNumSegments::ScoreToProportionalNumSegments(
    SaxSegIndT num_channels_avg, MtsNumChannelsT num_channels,
    const std::function<sptr<ISegmentationStrategy>(SaxSegIndT)> &strategy_factory, Real score_exponent)
    : m_num_segments_total(U(num_channels_avg * num_channels)),
      m_strategy_factory(std::move(strategy_factory)),
      m_score_exponent(score_exponent) {}

vec<sptr<ISegmentationStrategy>> ScoreToProportionalNumSegments::get_segmentation_strategies(
    const vec<Real> &scores) const {
    Real score_total = 0.0;
    vec<Real> exp_scores(scores);
    for (Real &exp_score : exp_scores) {
        exp_score = std::pow(exp_score, m_score_exponent);
        score_total += exp_score;
    }

    uint num_segments_remaining = m_num_segments_total;
    vec<sptr<ISegmentationStrategy>> segmentation_strategies;
    segmentation_strategies.reserve(exp_scores.size());

    for (uint i = 0; i < exp_scores.size(); ++i) {
        Real exp_score = exp_scores[i];
        SaxSegIndT num_segments =
            static_cast<SaxSegIndT>(std::round(exp_score / score_total * R(num_segments_remaining)));
        num_segments = std::max(
            static_cast<SaxSegIndT>(1),
            std::min(static_cast<SaxSegIndT>(num_segments_remaining - exp_scores.size() + i + 1), num_segments));
        segmentation_strategies.push_back(m_strategy_factory(num_segments));
        num_segments_remaining -= num_segments;
        score_total -= exp_score;
    }
    return segmentation_strategies;
}
