#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_WIDTHBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_WIDTHBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSegmentationStrategy.hpp"

class IScoreToSegmentationStrategy;
class EnvelopeWidthScores;

class EnvWidthChSegmentationStrategy : public ScoreBasedChSegmentationStrategy {
   public:
    /**
     * @brief Constructor for EnvWidthChSegmentationStrategy
     * @param score_to_segmentation_strategy Strategy to convert range-based scores to segmentation strategies
     * @param sampling_params Parameters for sampling channels and calculating mean ranges
     * @param min_mean_update Minimum update to the mean range as a proportion of max-min range. If in one update step,
     * no channel's mean range is updated by at least this amount, the sampling stops.
     */
    EnvWidthChSegmentationStrategy(uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy,
                                   SamplingChSSSamplingParams sampling_params, Real min_mean_update = 0.0);

    EnvWidthChSegmentationStrategy() = default;

    ChannelSegmentationStrategyType get_type() const override;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_WIDTHBASEDCHSEGMENTATIONSTRATEGY_HPP
