#include "Modules/Indexing/GetEntryGenerator.hpp"

#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/EntryGenerator/PaaEntryGenerator.hpp"
#include "Index/EntryGenerator/SaxMergingPaaEntryGenerator.hpp"
#include "Util/RunSettings/RunSettings.hpp"

uptr<IEntryGenerator<Paa>> get_paa_generator(const IndexOptions &opts,
                                             const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) {
    PaaParams paa_params = {
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
        .m_lg_segmentation_strategy = lg_segmentation_strategy,
    };
    uint num_len_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;

    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    if (index_params && index_params->m_merger_params.m_entry_merger_type == SAX_PAA_GENERATOR) {
        auto merger_sax_params = index_params->m_merger_params.m_merger_sax_params;
        if (!merger_sax_params || merger_sax_params->m_num_bits == 0) {
            throw std::runtime_error("SAX-merging PAA generator requires positive number of bits");
        }
        return std::make_unique<SaxMergingPaaEntryGenerator>(paa_params, merger_sax_params->m_num_bits, num_len_groups);
    }
    return std::make_unique<PaaEntryGenerator>(paa_params, num_len_groups);
}

uptr<IEntryGenerator<Envelope>> get_envelope_generator(
    const IndexOptions &opts, const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) {
    auto *params = dynamic_cast<const EnvelopeIndexParams *>(opts.m_index_params.get());

    auto &length_props = RunSettings::get_instance().get_length_props();
    return std::make_unique<EnvelopeEntryGenerator>(opts.m_normalized, params->m_pos_per_env, length_props,
                                                    lg_segmentation_strategy);
}
