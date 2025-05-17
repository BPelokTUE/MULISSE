#include "Modules/Indexing/IndexFactory/GetTwoStageEnvelopeIndex.hpp"

#include "Index/ChainIndex/ChainIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetISaxIndex.hpp"

sptr<IIndex<Envelope>> get_two_stage_isax_envelope_index(IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;

    vec<sptr<IIndex<Envelope>>> approx_indexes(1);
    approx_indexes[0] = get_isax_index<Envelope>(factory_params);
    if (discretize_flat_index) {
        auto isax_index_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.m_index_params.get());
        isax_index_params->m_sax_params.m_num_bits = isax_index_params->m_isax_trie_params.m_num_bits_limit;
    }
    auto exact_index = get_flat_envelope_index(factory_params);

    auto *index = new ChainIndex<Envelope>(std::move(approx_indexes), std::move(exact_index));
    return sptr<IIndex<Envelope>>(index);
}
