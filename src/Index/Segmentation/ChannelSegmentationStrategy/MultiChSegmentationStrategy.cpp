#include "Index/Segmentation/ChannelSegmentationStrategy/MultiChSegmentationStrategy.hpp"

#include <filesystem>
#include <fstream>

#include "Util/HelperFuncs/Conversion.hpp"

MultiChSegmentationStrategy::MultiChSegmentationStrategy(vec<sptr<ISegmentationStrategy>> segmentation_strategies)
    : m_segmentation_strategies(segmentation_strategies) {}

MultiChSegmentationStrategy::MultiChSegmentationStrategy(
    std::function<sptr<ISegmentationStrategy>(SaxSegIndT)> num_seg_to_strategy, SaxSegIndT avg_num_segments,
    const str num_seg_ratios_path) {
    if (!std::filesystem::exists(num_seg_ratios_path)) {
        throw std::runtime_error("File not found: " + num_seg_ratios_path);
    }

    std::ifstream ratios_ifs(num_seg_ratios_path);
    if (!ratios_ifs.is_open()) {
        throw std::runtime_error("Failed to open file: " + num_seg_ratios_path);
    }

    vec<Real> num_seg_ratios;
    Real num_seg_ratios_sum = 0.0, act_num_seg_ratio;

    while (ratios_ifs >> act_num_seg_ratio) {
        assert(act_num_seg_ratio > 0.0 && "Segment ratio must be positive");
        num_seg_ratios.push_back(act_num_seg_ratio);
        num_seg_ratios_sum += act_num_seg_ratio;
    }

    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(num_seg_ratios.size());
    uint num_seg_remaining = U(avg_num_segments * num_channels);

    m_segmentation_strategies.reserve(num_channels);
    for (Real num_seg_ratio : num_seg_ratios) {
        SaxSegIndT num_segments =
            static_cast<SaxSegIndT>(std::max(num_seg_ratio * R(num_seg_remaining) / num_seg_ratios_sum, R(1.0)));
        m_segmentation_strategies.push_back(num_seg_to_strategy(num_segments));
        num_seg_remaining -= U(num_segments);
        num_seg_ratios_sum -= num_seg_ratio;
    }
}

sptr<ISegmentationStrategy> MultiChSegmentationStrategy::get_segmentation_strategy(uint channel_ind) const {
    assert(channel_ind < m_segmentation_strategies.size() && "Channel index out of bounds");
    return m_segmentation_strategies[channel_ind];
}

const ISegmentationStrategy *MultiChSegmentationStrategy::get_const_segmentation_strategy(uint channel_ind) const {
    assert(channel_ind < m_segmentation_strategies.size() && "Channel index out of bounds");
    return m_segmentation_strategies[channel_ind].get();
}

ChannelSegmentationStrategyType MultiChSegmentationStrategy::get_type() const {
    return ChannelSegmentationStrategyType::MULTI;
}
