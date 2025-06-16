#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Entry/SaxEnvelope.hpp"
#include "Index/EnvelopeIndex/Flat/FlatEnvelopeIndex.hpp"

template <>
sptr<IIndex<Envelope>> get_flat_envelope_index<Envelope>(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());

    return std::make_shared<FlatEnvelopeIndex<Envelope>>(factory_params.m_ch_segmentation_strategy,
                                                         index_params->m_pos_per_env);
}

template <>
sptr<IIndex<Envelope>> get_flat_envelope_index<SaxEnvelope>(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());

    return std::make_shared<FlatEnvelopeIndex<SaxEnvelope>>(factory_params.m_ch_segmentation_strategy,
                                                            index_params->m_pos_per_env);
}
