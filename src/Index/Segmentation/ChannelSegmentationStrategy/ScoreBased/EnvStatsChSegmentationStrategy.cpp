#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvStatsChSegmentationStrategy.hpp"

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSSSamplingParams.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeStatsScores.hpp"
#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToSegmentationStrategy.hpp"
#include "Util/Stats/ScoreFunc/IndexStatsScoreFunc.hpp"

EnvStatsChSegmentationStrategy::EnvStatsChSegmentationStrategy(
    MtsNumChannelsT num_channels, uptr<IndexStatsScoreFunc> index_stats_score_func,
    uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy, SamplingChSSSamplingParams sampling_params)
    : ScoreBasedChSegmentationStrategy(
          std::make_unique<EnvelopeStatsScores>(num_channels, std::move(index_stats_score_func)),
          std::move(score_to_segmentation_strategy), std::move(sampling_params)) {}

ChannelSegmentationStrategyType EnvStatsChSegmentationStrategy::get_type() const {
    return ChannelSegmentationStrategyType::ENV_STATS_BASED;
}
