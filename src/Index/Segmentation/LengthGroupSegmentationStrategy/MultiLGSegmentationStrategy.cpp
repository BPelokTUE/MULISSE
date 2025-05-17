#include "Index/Segmentation/LengthGroupSegmentationStrategy/MultiLGSegmentationStrategy.hpp"

#include "Util/RunSettings/RunSettings.hpp"

MultiLGSegmentationStrategy::MultiLGSegmentationStrategy(
    std::function<sptr<IChannelSegmentationStrategy>(uint, uint)> ch_segmentation_strategy_factory) {
    auto &RS = RunSettings::get_instance();
    auto &length_props = RS.get_length_props();

    if (!length_props.m_use_length_groups) {
        throw std::runtime_error("MultiLGSegmentationStrategy requires length-based grouping.");
    }

    m_ch_segmentation_strategies.reserve(length_props.m_num_l_groups);
    for (uint lg_ind = 0; lg_ind < length_props.m_num_l_groups; ++lg_ind) {
        uint lg_l_min = RS.get_lg_l_min(lg_ind);
        uint lg_l_max = RS.get_lg_l_max(lg_ind);
        m_ch_segmentation_strategies.push_back(ch_segmentation_strategy_factory(lg_l_min, lg_l_max));
    }
}

sptr<IChannelSegmentationStrategy> MultiLGSegmentationStrategy::get_ch_segmentation_strategy(uint lg_ind) const {
    return m_ch_segmentation_strategies[lg_ind];
}

const IChannelSegmentationStrategy *MultiLGSegmentationStrategy::get_const_ch_segmentation_strategy(uint lg_ind) const {
    return m_ch_segmentation_strategies[lg_ind].get();
}

LengthGroupSegmentationStrategyType MultiLGSegmentationStrategy::get_type() const { return MULTI; }
