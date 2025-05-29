#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvWidthChSegmentationStrategy.hpp"

#include <algorithm>
#include <random>

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSSSamplingParams.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeWidthScores.hpp"
#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToSegmentationStrategy.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Stats/EnvelopeStatsUtil.hpp"

EnvWidthChSegmentationStrategy::EnvWidthChSegmentationStrategy(
    MtsNumChannelsT num_channels, uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy,
    SamplingChSSSamplingParams sampling_params, Real min_width_update)
    : ScoreBasedChSegmentationStrategy(std::make_unique<EnvelopeWidthScores>(num_channels, min_width_update),
                                       std::move(score_to_segmentation_strategy), std::move(sampling_params)) {}

ChannelSegmentationStrategyType EnvWidthChSegmentationStrategy::get_type() const {
    return ChannelSegmentationStrategyType::ENV_WIDTH_BASED;
}
