#include "Modules/Indexing/IndexFactory/GetTwoStageEnvelopeIndex.hpp"

#include "Index/ChainIndex/ChainIndex.hpp"
#include "Index/Entry/SaxEnvelope.hpp"
#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetISaxIndex.hpp"

template <>
sptr<IIndex<Envelope>> get_two_stage_isax_envelope_index<Envelope>(IndexFactoryParams &factory_params) {
    vec<sptr<IIndex<Envelope>>> approx_indexes(1);
    approx_indexes[0] = get_isax_index<Envelope>(factory_params);
    auto exact_index = get_flat_envelope_index<Envelope>(factory_params);

    return std::make_shared<ChainIndex<Envelope>>(std::move(approx_indexes), std::move(exact_index));
}

template <>
sptr<IIndex<Envelope>> get_two_stage_isax_envelope_index<SaxEnvelope>(IndexFactoryParams &factory_params) {
    vec<sptr<IIndex<Envelope>>> approx_indexes(1);
    approx_indexes[0] = get_isax_index<Envelope>(factory_params);
    auto exact_index = get_flat_envelope_index<SaxEnvelope>(factory_params);

    return std::make_shared<ChainIndex<Envelope>>(std::move(approx_indexes), std::move(exact_index));
}
