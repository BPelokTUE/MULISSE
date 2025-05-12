#ifndef INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_ADAPTIVEMULTISEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_ADAPTIVEMULTISEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/LengthGroupSegmentationStrategy/MultiLGSegmentationStrategy.hpp"

/** @brief ILengthGroupSegmentationStrategy implementation that uses different SegmentationStrategy for each group,
 * parametrized by the length range of the group, and adapted based on the total "presence" of the group. */
class AdaptiveMultiLGSegmentationStrategy : public MultiLGSegmentationStrategy {
   public:
    /**
     * @brief Construct a new AdaptiveMultiLGSegmentationStrategy with the given segmentation strategy factory.
     * @param segmentation_strategy_factory The factory function to create segmentation strategies for each group.
     * @param avg_num_segments The average number of segments for each group.
     * @param pos_per_env The number of positions per envelope.
     */
    AdaptiveMultiLGSegmentationStrategy(
        std::function<sptr<ISegmentationStrategy>(uint, uint, SaxSegIndT)> segmentation_strategy_factory,
        SaxSegIndT avg_num_segments, uint pos_per_env);

    LengthGroupSegmentationStrategyType get_type() const override;

   private:
    /**
     * @brief Calculate the number of segments for each length group
     * @param length_props Length properties
     * @param pos_per_env Number of positions per envelope
     * @param avg_num_segments Average number of segments
     */
    vec<SaxSegIndT> calculate_num_segments_per_lg(const LengthProperties &length_props, uint pos_per_env,
                                                  SaxSegIndT avg_num_segments);
};

#endif  // INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_ADAPTIVEMULTISEGMENTATIONSTRATEGY_HPP
