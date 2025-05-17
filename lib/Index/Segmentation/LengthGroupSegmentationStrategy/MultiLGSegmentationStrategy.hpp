#ifndef INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_MULTISEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_MULTISEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"

/** @brief ILengthGroupSegmentationStrategy implementation that uses different SegmentationStrategy for each group,
 * parametrized by the length range of the group. */
class MultiLGSegmentationStrategy : public ILengthGroupSegmentationStrategy {
   public:
    /**
     * @brief Construct a new MultiLGSegmentationStrategy with the given a segmentation strategy factory.
     * @param ch_segmentation_strategy_factory The factory function to create channel segmentation strategies for each
     * group.
     */
    MultiLGSegmentationStrategy(
        std::function<sptr<IChannelSegmentationStrategy>(uint, uint)> ch_segmentation_strategy_factory);

    MultiLGSegmentationStrategy() = default;

    sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(uint lg_ind) const override;

    const IChannelSegmentationStrategy *get_const_ch_segmentation_strategy(uint lg_ind) const override;

    LengthGroupSegmentationStrategyType get_type() const override;

   protected:
    vec<sptr<IChannelSegmentationStrategy>> m_ch_segmentation_strategies;
};

#endif  // INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_MULTISEGMENTATIONSTRATEGY_HPP
