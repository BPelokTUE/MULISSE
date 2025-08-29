#include "Index/Estimator/IndexSizeEstimator.hpp"

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/RunSettings/RunSettings.hpp"

using CHSS = ChannelSegmentationStrategyType;

IndexSizeEstimator::IndexSizeEstimator(IndexType method_type, LengthProperties length_props,
                                       const ILengthGroupSegmentationStrategy *lg_segmentation_strategy,
                                       SaxSegIndT num_segments)
    : m_lg_segmentation_strategy(lg_segmentation_strategy),
      m_length_props(length_props),
      m_segment_len(num_segments > 0 ? length_props.m_l_max / num_segments : 0) {
    switch (method_type) {
        case ENVELOPE:
            m_size_per_bound = sizeof(Real);
            break;
        case SAX_ENVELOPE:
            m_size_per_bound = sizeof(SaxSymbolT);
            break;
        default:
            throw std::invalid_argument(
                "IndexSizeEstimator can only be used with methods that have an estimable size.");
    }
}

size_t IndexSizeEstimator::get_estimated_flat_envelope_size(uint pos_per_env, bool add_entry_vec_size) {
    auto [num_channels, series_len, num_series, ds_file] = RunSettings::get_instance().get_dataset_props();

    size_t estimated_size = 0;
    for (uint lg_ind = 0; lg_ind < m_length_props.m_num_l_groups; ++lg_ind) {
        uint l_min = m_length_props.get_lg_l_min(lg_ind), l_max = m_length_props.get_lg_l_max(lg_ind);
        uint num_envelopes = (series_len - l_min + pos_per_env) / pos_per_env * num_series;
        const IChannelSegmentationStrategy *ch_segmentation_strategy =
            m_lg_segmentation_strategy ? m_lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind)
                                       : nullptr;

        size_t num_segments_total = 0;
        for (MtsNumChannelsT ch_ind = 0; ch_ind < num_channels; ++ch_ind) {
            SaxSegIndT ch_num_segments =
                ch_segmentation_strategy
                    ? ch_segmentation_strategy->get_const_segmentation_strategy(ch_ind)->get_num_segments(l_max)
                    : static_cast<SaxSegIndT>(l_max / m_segment_len);
            num_segments_total += static_cast<size_t>(ch_num_segments);
        }
        // The estimated size of an IndexEntry<Envelope> is:
        // - subsequence information: 3 * sizeof(uint) = 12
        // - The size of the multivariate time series summary vector (i.e. number of channels) = 8
        // - The size of the lower and upper bound vectors for each channel = 2 * num_channels * sizeof(size_t) = 16 *
        // num_channels
        // - The lower and upper bound vectors = 2 * num_segments_total * sizeof(Real)
        // => 20 + 16 * num_channels + 2 * num_segments_total * sizeof(Real)
        estimated_size += num_envelopes * (sizeof(SubsequenceInfo) + sizeof(size_t) +
                                           2 * (num_segments_total * m_size_per_bound + num_channels * sizeof(size_t)));

        if (add_entry_vec_size) estimated_size += get_overhead_size();
    }
    return estimated_size;
}

uint IndexSizeEstimator::get_min_pos_per_env(Real index_size_limit) {
    auto &RS = RunSettings::get_instance();

    uint series_len = RS.get_dataset_props().m_series_len;

    size_t bytes_limit = static_cast<size_t>(index_size_limit * R(get_dataset_size(RS.get_dataset_path())));

    size_t numerator = get_estimated_flat_envelope_size(1, false);
    size_t min_size = get_estimated_flat_envelope_size(series_len, false);
    for (uint lg_ind = 0; lg_ind < m_length_props.m_num_l_groups; ++lg_ind) min_size += get_overhead_size();
    size_t denominator = min_size < bytes_limit ? bytes_limit - min_size : 1;

    uint min_pos_per_env = RS.get_envelope_props().m_pos_per_env;
    uint max_pos_per_env = m_length_props.m_l_max - m_length_props.m_l_min + 1;
    return std::min(max_pos_per_env,
                    std::max(min_pos_per_env, static_cast<uint>((numerator + denominator - 1) / denominator)));
}

size_t IndexSizeEstimator::get_overhead_size() { return 107; }
