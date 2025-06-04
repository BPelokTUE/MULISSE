#include "Util/Stats/IndexSizeEstimator.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/RunSettings/RunSettings.hpp"

size_t get_estimated_flat_envelope_size(const ILengthGroupSegmentationStrategy *lg_segmentation_strategy,
                                        uint pos_per_env) {
    auto &RS = RunSettings::get_instance();

    uint num_len_groups = RS.get_length_props().m_num_l_groups;
    auto [num_channels, series_len, num_series, ds_file] = RS.get_dataset_props();

    size_t estimated_size = 0;
    for (uint lg_ind = 0; lg_ind < num_len_groups; ++lg_ind) {
        uint l_min = RS.get_lg_l_min(lg_ind), l_max = RS.get_lg_l_max(lg_ind);
        uint num_envelopes = (series_len - l_min + pos_per_env) / pos_per_env;
        const IChannelSegmentationStrategy *ch_segmentation_strategy =
            lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind);

        size_t num_segments_total = 0;
        for (MtsNumChannelsT ch_ind = 0; ch_ind < num_channels; ++ch_ind) {
            SaxSegIndT num_segments =
                ch_segmentation_strategy->get_const_segmentation_strategy(ch_ind)->get_num_segments(l_max);
            num_segments_total += static_cast<size_t>(num_segments);
        }
        estimated_size += num_envelopes * (num_segments_total * 2 * sizeof(Real) + sizeof(SubsequenceInfo));
    }
    return estimated_size * num_series;
}

uint get_max_pos_per_env(Real index_size_limit, const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) {
    auto &RS = RunSettings::get_instance();

    size_t bytes_limit =
        static_cast<size_t>(index_size_limit * R(get_dataset_size(RunSettings::get_instance().get_dataset_path())));

    size_t numerator = get_estimated_flat_envelope_size(lg_segmentation_strategy, 1);
    size_t min_size = get_estimated_flat_envelope_size(lg_segmentation_strategy, RS.get_dataset_props().m_series_len);
    size_t denominator = min_size < bytes_limit ? bytes_limit - min_size : 1;

    uint min_pos_per_env = RS.get_envelope_props().m_pos_per_env;
    uint max_pos_per_env = RS.get_length_props().m_l_max - RS.get_length_props().m_l_min + 1;

    return std::min(max_pos_per_env,
                    std::max(min_pos_per_env, static_cast<uint>((numerator + denominator - 1) / denominator)));
}
