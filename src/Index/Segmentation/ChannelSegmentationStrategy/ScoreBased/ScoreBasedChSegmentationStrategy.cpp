#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSegmentationStrategy.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreToSegmentationStrategy.hpp"

ScoreBasedChSegmentationStrategy::~ScoreBasedChSegmentationStrategy() = default;

ScoreBasedChSegmentationStrategy::ScoreBasedChSegmentationStrategy() = default;

ScoreBasedChSegmentationStrategy::ScoreBasedChSegmentationStrategy(
    const vec<Real> &channel_scores, uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy)
    : MultiChSegmentationStrategy(score_to_segmentation_strategy->get_segmentation_strategies(channel_scores)) {}
