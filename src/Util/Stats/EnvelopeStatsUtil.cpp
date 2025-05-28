#include "Util/Stats/EnvelopeStatsUtil.hpp"

#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"

uptr<EnvelopeEntryGenerator> get_simple_envelope_entry_generator(uint segment_len) {
    auto &RS = RunSettings::get_instance();
    uint series_len = RS.get_dataset_props().m_series_len;

    auto &length_props = RS.get_length_props();
    uint l_min = length_props.m_l_min, l_max = length_props.m_l_max;
    auto segmentation_strategy = std::make_shared<UniformSegmentationStrategy>(l_max, l_max / segment_len);
    auto ch_segmentation_strategy = std::make_shared<SingleChSegmentationStrategy>(std::move(segmentation_strategy));

    uint pos_per_env = series_len - l_min + 1;
    auto lg_segmentation_strategy = std::make_unique<SingleLGSegmentationStrategy>(std::move(ch_segmentation_strategy));
    EnvelopeParams envelope_params{.m_l_min = l_min,
                                   .m_l_max = l_max,
                                   .m_pos_per_env = pos_per_env,
                                   .m_lg_segmentation_strategy = lg_segmentation_strategy.get()};

    return std::make_unique<EnvelopeEntryGenerator>(true, envelope_params);
}
