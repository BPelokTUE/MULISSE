#ifndef INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_SINGLESEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_SINGLESEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"

/** @brief ILengthGroupSegmentationStrategy implementation that uses the same SegmentationStrategy for all groups. */
class SingleLGSegmentationStrategy : public ILengthGroupSegmentationStrategy {
   public:
    /**
     * @brief Construct a new SingleLGSegmentationStrategy with the given segmentation strategy.
     * @param ch_segmentation_strategy The channel segmentation strategy to use for all groups.
     */
    SingleLGSegmentationStrategy(sptr<IChannelSegmentationStrategy> &&ch_segmentation_strategy);

    sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(uint lg_ind) const override;

    const IChannelSegmentationStrategy *get_const_ch_segmentation_strategy(uint lg_ind) const override;

    LengthGroupSegmentationStrategyType get_type() const override;

   private:
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
};

#endif  // INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_SINGLESEGMENTATIONSTRATEGY_HPP
