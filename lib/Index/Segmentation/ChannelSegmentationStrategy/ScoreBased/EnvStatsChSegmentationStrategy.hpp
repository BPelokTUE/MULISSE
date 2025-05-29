#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVSTATSCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVSTATSCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSegmentationStrategy.hpp"

class IndexStatsScoreFunc;
class IScoreToSegmentationStrategy;

class EnvStatsChSegmentationStrategy : public ScoreBasedChSegmentationStrategy {
   public:
    /**
     * @brief Constructor for WidthBasedChSegmentationStrategy
     * @param num_channels Number of channels
     * @param score_to_segmentation_strategy Strategy to convert range-based scores to segmentation strategies
     * @param index_stats_score_func Function to calculate index statistics scores
     * @param sampling_params Parameters for sampling channels and calculating mean ranges
     */
    EnvStatsChSegmentationStrategy(MtsNumChannelsT num_channels, uptr<IndexStatsScoreFunc> index_stats_score_func,
                                   uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy,
                                   SamplingChSSSamplingParams sampling_params);

    EnvStatsChSegmentationStrategy() = default;

    ChannelSegmentationStrategyType get_type() const override;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVSTATSCHSEGMENTATIONSTRATEGY_HPP
