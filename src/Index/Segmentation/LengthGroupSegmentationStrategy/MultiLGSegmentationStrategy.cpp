#include "Index/Segmentation/LengthGroupSegmentationStrategy/MultiLGSegmentationStrategy.hpp"

#include "Util/RunSettings/RunSettings.hpp"

MultiLGSegmentationStrategy::MultiLGSegmentationStrategy(
    std::function<sptr<ISegmentationStrategy>(uint, uint)> segmentation_strategy_factory) {
    auto &RS = RunSettings::get_instance();
    auto &length_props = RS.get_length_props();

    if (!length_props.m_use_length_groups) {
        throw std::runtime_error("MultiLGSegmentationStrategy requires length-based grouping.");
    }

    m_segmentation_strategies.reserve(length_props.m_num_l_groups);
    for (uint lg_ind = 0; lg_ind < length_props.m_num_l_groups; ++lg_ind) {
        uint lg_l_min = RS.get_lg_l_min(lg_ind);
        uint lg_l_max = RS.get_lg_l_max(lg_ind);
        m_segmentation_strategies.push_back(segmentation_strategy_factory(lg_l_min, lg_l_max));
    }
}

sptr<ISegmentationStrategy> MultiLGSegmentationStrategy::get_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategies[lg_ind];
}

const ISegmentationStrategy *MultiLGSegmentationStrategy::get_const_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategies[lg_ind].get();
}

LengthGroupSegmentationStrategyType MultiLGSegmentationStrategy::get_type() const { return MULTI; }
