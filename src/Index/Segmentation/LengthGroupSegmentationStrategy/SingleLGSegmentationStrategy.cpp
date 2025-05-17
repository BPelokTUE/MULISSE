#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"

SingleLGSegmentationStrategy::SingleLGSegmentationStrategy(sptr<IChannelSegmentationStrategy> &&segmentation_strategy)
    : m_ch_segmentation_strategy(std::move(segmentation_strategy)) {}

sptr<IChannelSegmentationStrategy> SingleLGSegmentationStrategy::get_ch_segmentation_strategy(uint lg_ind) const {
    return m_ch_segmentation_strategy;
}

const IChannelSegmentationStrategy *SingleLGSegmentationStrategy::get_const_ch_segmentation_strategy(
    uint lg_ind) const {
    return m_ch_segmentation_strategy.get();
}

LengthGroupSegmentationStrategyType SingleLGSegmentationStrategy::get_type() const { return SINGLE; }
