#ifndef INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_MULTISEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_MULTISEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"

/** @brief ILengthGroupSegmentationStrategy implementation that uses different SegmentationStrategy for each group,
 * parametrized by the length range of the group. */
class MultiLGSegmentationStrategy : public ILengthGroupSegmentationStrategy {
   public:
    /**
     * @brief Construct a new MultiLGSegmentationStrategy with the given a segmentation strategy factory.
     * @param segmentation_strategy_factory The factory function to create segmentation strategies for each group.
     */
    MultiLGSegmentationStrategy(std::function<sptr<ISegmentationStrategy>(uint, uint)> segmentation_strategy_factory);

    MultiLGSegmentationStrategy() = default;

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint lg_ind) const override;

    const ISegmentationStrategy *get_const_segmentation_strategy(uint lg_ind) const override;

    LengthGroupSegmentationStrategyType get_type() const override;

   protected:
    vec<sptr<ISegmentationStrategy>> m_segmentation_strategies;
};

#endif  // INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_MULTISEGMENTATIONSTRATEGY_HPP
