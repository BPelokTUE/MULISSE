#ifndef INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_SINGLESEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_SINGLESEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"

/** @brief ILengthGroupSegmentationStrategy implementation that uses the same SegmentationStrategy for all groups. */
class SingleLGSegmentationStrategy : public ILengthGroupSegmentationStrategy {
   public:
    /**
     * @brief Construct a new SingleLGSegmentationStrategy with the given segmentation strategy.
     * @param segmentation_strategy The segmentation strategy to use for all groups.
     */
    SingleLGSegmentationStrategy(sptr<ISegmentationStrategy> &&segmentation_strategy);

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint lg_ind) const override;

    const ISegmentationStrategy *get_const_segmentation_strategy(uint lg_ind) const override;

    LengthGroupSegmentationStrategyType get_type() const override;

   private:
    sptr<ISegmentationStrategy> m_segmentation_strategy;
};

#endif  // INDEX_SEGMENTATION_LENGHTGROUPSEGMENTATIONSTRATEGY_SINGLESEGMENTATIONSTRATEGY_HPP
