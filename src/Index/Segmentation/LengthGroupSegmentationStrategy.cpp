#include "Index/Segmentation/LengthGroupSegmentationStrategy.hpp"

#include "Index/Segmentation/Presence.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

// SingleSegmentationStrategy

SingleSegmentationStrategy::SingleSegmentationStrategy(sptr<ISegmentationStrategy> &&segmentation_strategy)
    : m_segmentation_strategy(std::move(segmentation_strategy)) {}

sptr<ISegmentationStrategy> SingleSegmentationStrategy::get_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategy;
}

const ISegmentationStrategy *SingleSegmentationStrategy::get_const_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategy.get();
}

LengthGroupSegmentationStrategyType SingleSegmentationStrategy::get_type() const { return SINGLE; }

// MultiSegmentationStrategy

MultiSegmentationStrategy::MultiSegmentationStrategy(
    std::function<sptr<ISegmentationStrategy>(uint, uint)> segmentation_strategy_factory) {
    auto &RS = RunSettings::get_instance();
    auto &length_props = RS.get_length_props();

    if (!length_props.m_use_length_groups) {
        throw std::runtime_error("MultiSegmentationStrategy requires length-based grouping.");
    }

    m_segmentation_strategies.reserve(length_props.m_num_l_groups);
    for (uint lg_ind = 0; lg_ind < length_props.m_num_l_groups; ++lg_ind) {
        uint lg_l_min = RS.get_lg_l_min(lg_ind);
        uint lg_l_max = RS.get_lg_l_max(lg_ind);
        m_segmentation_strategies.push_back(segmentation_strategy_factory(lg_l_min, lg_l_max));
    }
}

sptr<ISegmentationStrategy> MultiSegmentationStrategy::get_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategies[lg_ind];
}

const ISegmentationStrategy *MultiSegmentationStrategy::get_const_segmentation_strategy(uint lg_ind) const {
    return m_segmentation_strategies[lg_ind].get();
}

LengthGroupSegmentationStrategyType MultiSegmentationStrategy::get_type() const { return MULTI; }

// AdaptiveMultiSegmentationStrategy

AdaptiveMultiSegmentationStrategy::AdaptiveMultiSegmentationStrategy(
    std::function<sptr<ISegmentationStrategy>(uint, uint, SaxSegIndT)> segmentation_strategy_factory,
    SaxSegIndT avg_num_segments, uint pos_per_env) {
    auto &RS = RunSettings::get_instance();
    auto &length_props = RS.get_length_props();

    if (!length_props.m_use_length_groups) {
        throw std::runtime_error("AdaptiveMultiSegmentationStrategy requires length-based grouping.");
    }

    m_segmentation_strategies.reserve(length_props.m_num_l_groups);
    auto num_segments_per_lg = calculate_num_segments_per_lg(length_props, pos_per_env, avg_num_segments);

    for (uint lg_ind = 0; lg_ind < length_props.m_num_l_groups; ++lg_ind) {
        uint lg_l_min = RS.get_lg_l_min(lg_ind);
        uint lg_l_max = RS.get_lg_l_max(lg_ind);
        m_segmentation_strategies.push_back(
            segmentation_strategy_factory(lg_l_min, lg_l_max, num_segments_per_lg[lg_ind]));
    }
}

vec<SaxSegIndT> AdaptiveMultiSegmentationStrategy::calculate_num_segments_per_lg(const LengthProperties &length_props,
                                                                                 uint pos_per_env,
                                                                                 SaxSegIndT avg_num_segments) {
    auto &RS = RunSettings::get_instance();
    uint series_len = RS.get_dataset_props().m_series_len;

    PresenceArray presence_array(length_props.m_l_min, length_props.m_l_max, series_len, pos_per_env);
    auto &presences = presence_array.get_presences();

    vec<SaxSegIndT> num_segments_per_lg(length_props.m_num_l_groups);
    size_t accounted_presence = 0, remaining_presence = presence_array.get_presence_sum();
    uint remaining_segments = U(avg_num_segments * length_props.m_num_l_groups);
    size_t presence_per_segment = remaining_presence / remaining_segments;

    for (uint lg_num = length_props.m_num_l_groups; lg_num > 0; --lg_num) {
        uint lg_ind = lg_num - 1, lg_l_min = RS.get_lg_l_min(lg_ind), lg_l_max = RS.get_lg_l_max(lg_ind);
        size_t lg_presence_sum = 0;

        for (uint l = lg_l_max; l >= lg_l_min; --l) {
            lg_presence_sum += presences[l] - accounted_presence;
        }
        lg_presence_sum += (presences[lg_l_min] - accounted_presence) * (lg_l_min - 1);
        accounted_presence += presences[lg_l_min] - accounted_presence;

        num_segments_per_lg[lg_ind] = static_cast<SaxSegIndT>(lg_presence_sum / presence_per_segment);

        remaining_presence -= lg_presence_sum;
        remaining_segments -= num_segments_per_lg[lg_ind];
        presence_per_segment = remaining_presence / remaining_segments;
    }
    return num_segments_per_lg;
}

LengthGroupSegmentationStrategyType AdaptiveMultiSegmentationStrategy::get_type() const { return ADAPTIVE_MULTI; }
