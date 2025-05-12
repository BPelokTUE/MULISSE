#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"

SingleLGSegmentationStrategy::SingleLGSegmentationStrategy(sptr<ISegmentationStrategy> &&segmentation_strategy)
    : m_segmentation_strategy(std::move(segmentation_strategy)) {}

sptr<ISegmentationStrategy> SingleLGSegmentationStrategy::get_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategy;
}

const ISegmentationStrategy *SingleLGSegmentationStrategy::get_const_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategy.get();
}

LengthGroupSegmentationStrategyType SingleLGSegmentationStrategy::get_type() const { return SINGLE; }