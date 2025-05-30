#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSegmentationStrategy.hpp"

#include <algorithm>
#include <random>

#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSSSamplingParams.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

SamplingChSSLateInitParams::~SamplingChSSLateInitParams() = default;

void SamplingChSegmentationStrategy::initialize(SamplingChSSSamplingParams sampling_params) {
    // 1. Get dataset path
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();

    // 2. Get subset of dataset
    auto dataset_props = RS.get_dataset_props();
    MtsNumChannelsT num_channels = dataset_props.m_num_channels;
    uint series_len = dataset_props.m_series_len, num_series = dataset_props.m_num_series;

    vec<vec<uint>> mts_inds(num_channels, vec<uint>(sampling_params.m_sample_size));
    auto mt = std::mt19937{std::random_device{}()};
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        vec<uint> channel_inds(num_series);
        std::iota(channel_inds.begin(), channel_inds.end(), 0);
        std::shuffle(channel_inds.begin(), channel_inds.end(), mt);
        mts_inds[c].assign(channel_inds.begin(), channel_inds.begin() + sampling_params.m_sample_size);
    }

    // 3. Get generator

    auto &length_props = RS.get_length_props();
    uint l_min = length_props.m_l_min, l_max = length_props.m_l_max;
    auto segmentation_strategy =
        std::make_shared<UniformSegmentationStrategy>(l_max, l_max / sampling_params.m_segment_len);
    auto ch_segmentation_strategy = std::make_shared<SingleChSegmentationStrategy>(std::move(segmentation_strategy));

    uint pos_per_env = series_len - l_min + 1;
    auto lg_segmentation_strategy = std::make_unique<SingleLGSegmentationStrategy>(std::move(ch_segmentation_strategy));
    EnvelopeParams envelope_params{.m_l_min = l_min,
                                   .m_l_max = l_max,
                                   .m_pos_per_env = pos_per_env,
                                   .m_lg_segmentation_strategy = lg_segmentation_strategy.get()};

    auto generator = std::make_unique<EnvelopeEntryGenerator>(sampling_params.m_normalized, envelope_params);

    // 4. Set late initialization parameters
    m_late_init_params =
        std::make_unique<SamplingChSSLateInitParams>(num_channels, sampling_params.m_sample_size, series_len,
                                                     &dataset_path, std::move(mts_inds), std::move(generator));

    // 5. Initialize segmentation strategies according to the implementation
    RS.set_lengths_per_group(l_max - l_min + 1);  // Ugly hack
    initialize_segmentation_strategies();
    RS.set_lengths_per_group(length_props.m_l_per_group);  // Ugly hack
}
