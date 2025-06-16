#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"

#include "Index/Entry/SaxEnvelope.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <typename EnvT>
uint FinalizedFlatEnvelopeIndex<EnvT>::size() const {
    return U(m_entries.size());
}

// Envelope

template class FinalizedFlatEnvelopeIndex<Envelope>;

template <>
FinalizedFlatEnvelopeIndex<Envelope>::FinalizedFlatEnvelopeIndex() = default;

template <>
FinalizedFlatEnvelopeIndex<Envelope>::FinalizedFlatEnvelopeIndex(
    sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env,
    vec<IndexEntry<Envelope>> &&entries)
    : FinalizedEnvelopeIndex(ch_segmentation_strategy, pos_per_env), m_entries(std::move(entries)) {}

// SaxEnvelope

template class FinalizedFlatEnvelopeIndex<SaxEnvelope>;

template <>
FinalizedFlatEnvelopeIndex<SaxEnvelope>::FinalizedFlatEnvelopeIndex() {
    m_breakpoints = &RunSettings::get_instance().get_breakpoints();
}

template <>
FinalizedFlatEnvelopeIndex<SaxEnvelope>::FinalizedFlatEnvelopeIndex(
    sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env,
    vec<IndexEntry<SaxEnvelope>> &&entries)
    : FinalizedEnvelopeIndex(ch_segmentation_strategy, pos_per_env), m_entries(std::move(entries)) {
    m_breakpoints = &RunSettings::get_instance().get_breakpoints();
}
