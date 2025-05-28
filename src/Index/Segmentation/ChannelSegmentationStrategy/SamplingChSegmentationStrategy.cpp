#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSegmentationStrategy.hpp"

#include <algorithm>
#include <random>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Stats/EnvelopeStatsUtil.hpp"

void SamplingChSegmentationStrategy::initialize(SamplingParams sampling_params) {
    // 1. Get dataset path
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();

    // 2. Get subset of dataset
    auto dataset_props = RS.get_dataset_props();
    MtsNumChannelsT num_channels = dataset_props.m_num_channels;
    uint series_len = dataset_props.m_series_len, num_series = dataset_props.m_num_series;
    uint subset_size = sampling_params.m_sample_size > 0 ? sampling_params.m_sample_size
                                                         : U(R(num_series) * sampling_params.m_sample_frac);

    vec<vec<uint>> mts_inds(num_channels, vec<uint>(subset_size));
    auto mt = std::mt19937{std::random_device{}()};
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        vec<uint> channel_inds(num_series);
        std::iota(channel_inds.begin(), channel_inds.end(), 0);
        std::shuffle(channel_inds.begin(), channel_inds.end(), mt);
        mts_inds[c].assign(channel_inds.begin(), channel_inds.begin() + subset_size);
    }

    // 3. Get generator
    auto generator = get_simple_envelope_entry_generator(sampling_params.m_segment_len);

    // 4. Set late initialization parameters
    m_late_init_params = std::make_unique<SamplingChSSLateInitParams>(SamplingChSSLateInitParams{
        num_channels, subset_size, series_len, &dataset_path, std::move(mts_inds), std::move(generator)});

    // 5. Initialize segmentation strategies according to the implementation
    auto &length_props = RS.get_length_props();
    RS.set_lengths_per_group(length_props.m_l_max - length_props.m_l_min + 1);  // Ugly hack
    initialize_segmentation_strategies();
    RS.set_lengths_per_group(length_props.m_l_per_group);  // Ugly hack
}
