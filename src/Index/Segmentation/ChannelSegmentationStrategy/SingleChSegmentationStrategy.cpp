#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"

#include "Enums/ChannelSegmentationStrategyType.hpp"

SingleChSegmentationStrategy::SingleChSegmentationStrategy(sptr<ISegmentationStrategy> segmentation_strategy)
    : m_segmentation_strategy(segmentation_strategy) {}

sptr<ISegmentationStrategy> SingleChSegmentationStrategy::get_segmentation_strategy(uint ch_ind) const {
    return m_segmentation_strategy;
}

const ISegmentationStrategy *SingleChSegmentationStrategy::get_const_segmentation_strategy(uint ch_ind) const {
    return m_segmentation_strategy.get();
}

ChannelSegmentationStrategyType SingleChSegmentationStrategy::get_type() const {
    return ChannelSegmentationStrategyType::SINGLE;
}
