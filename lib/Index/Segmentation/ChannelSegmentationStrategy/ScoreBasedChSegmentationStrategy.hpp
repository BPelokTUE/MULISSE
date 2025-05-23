#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

class IScoreToSegmentationStrategy;
class IndexStatsScoreFunc;

class ScoreBasedChSegmentationStrategy : public IChannelSegmentationStrategy {
   public:
    /**
     * @brief Constructor for ScoreBasedChSegmentationStrategy.
     * @param index_stats_score_function Function for calculating scores based on index (envelope) statistics
     * @param score_to_segmentation_strategy Function for converting scores to segmentation strategies
     * @param subset_fraction Fraction of the dataset to use for calculating index (envelope) statistics
     * @param segment_len Length of the segments used for calculating index (envelope) statistics
     */
    ScoreBasedChSegmentationStrategy(const IndexStatsScoreFunc* index_stats_score_function,
                                     const IScoreToSegmentationStrategy* score_to_segmentation_strategy,
                                     Real subset_fraction = R(0.01), SaxSegIndT segment_len = 1);

    ScoreBasedChSegmentationStrategy() = default;

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint channel_ind) const override;

    const ISegmentationStrategy* get_const_segmentation_strategy(uint channel_ind) const override;

    ChannelSegmentationStrategyType get_type() const override;

   private:
    vec<sptr<ISegmentationStrategy>> m_segmentation_strategies;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(m_segmentation_strategies);
    }
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
