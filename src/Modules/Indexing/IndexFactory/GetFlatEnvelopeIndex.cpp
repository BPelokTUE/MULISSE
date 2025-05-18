#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/Flat/FlatEnvelopeIndex.hpp"

sptr<IIndex<Envelope>> get_flat_envelope_index(IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());

    auto sax_index_params = dynamic_cast<SaxIndexParams *>(opts.m_index_params.get());
    if (discretize_flat_index && sax_index_params && sax_index_params->m_sax_params.m_num_bits > 0) {
        auto *index = new FlatEnvelopeIndex(factory_params.m_ch_segmentation_strategy, index_params->m_pos_per_env,
                                            sax_index_params->m_sax_params.m_num_bits);
        return sptr<IIndex<Envelope>>(index);
    } else {
        auto *index = new FlatEnvelopeIndex(factory_params.m_ch_segmentation_strategy, index_params->m_pos_per_env);
        return sptr<IIndex<Envelope>>(index);
    }
}
