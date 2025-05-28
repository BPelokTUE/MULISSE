#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_WIDTHBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_WIDTHBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSegmentationStrategy.hpp"

class IScoreToSegmentationStrategy;

class WidthBasedChSegmentationStrategy : public SamplingChSegmentationStrategy {
   public:
    /**
     * @brief Constructor for WidthBasedChSegmentationStrategy
     * @param score_to_segmentation_strategy Strategy to convert range-based scores to segmentation strategies
     * @param sampling_params Parameters for sampling channels and calculating mean ranges
     * @param min_mean_update Minimum update to the mean range as a proportion of max-min range. If in one update step,
     * no channel's mean range is updated by at least this amount, the sampling stops.
     */
    WidthBasedChSegmentationStrategy(const IScoreToSegmentationStrategy* score_to_segmentation_strategy,
                                     SamplingParams sampling_params, Real min_mean_update = 0.0);

    WidthBasedChSegmentationStrategy() = default;

    void initialize_segmentation_strategies() override;

    ChannelSegmentationStrategyType get_type() const override;

   private:
    const IScoreToSegmentationStrategy* m_score_to_segmentation_strategy;
    Real m_min_mean_update;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_WIDTHBASEDCHSEGMENTATIONSTRATEGY_HPP
