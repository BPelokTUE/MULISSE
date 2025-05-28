#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

class IScoreToSegmentationStrategy;
class IndexStatsScoreFunc;

class ScoreBasedChSegmentationStrategy : public SamplingChSegmentationStrategy {
   public:
    /**
     * @brief Constructor for ScoreBasedChSegmentationStrategy.
     * @param index_stats_score_function Function for calculating scores based on index (envelope) statistics
     * @param score_to_segmentation_strategy Function for converting scores to segmentation strategies
     * @param sampling_params Parameters for sampling
     */
    ScoreBasedChSegmentationStrategy(const IndexStatsScoreFunc *index_stats_score_function,
                                     const IScoreToSegmentationStrategy *score_to_segmentation_strategy,
                                     SamplingParams sampling_params);

    ScoreBasedChSegmentationStrategy() = default;

    ChannelSegmentationStrategyType get_type() const override;

   private:
    void initialize_segmentation_strategies() override;

    const IndexStatsScoreFunc *m_index_stats_score_func;

    const IScoreToSegmentationStrategy *m_score_to_segmentation_strategy;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
