#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

class IScoreToSegmentationStrategy;
class IndexStatsScoreFunc;

class ScoreBasedChSegmentationStrategy : public IChannelSegmentationStrategy {
   public:
    ScoreBasedChSegmentationStrategy(const IndexStatsScoreFunc* index_stats_score_function,
                                     const IScoreToSegmentationStrategy* score_to_segmentation_strategy,
                                     Real subset_fraction = R(0.01), SaxSegIndT segment_len = 1);

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint channel_ind) const override;

    const ISegmentationStrategy* get_const_segmentation_strategy(uint channel_ind) const override;

    ChannelSegmentationStrategyType get_type() const override;

   private:
    vec<sptr<ISegmentationStrategy>> m_segmentation_strategies;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
