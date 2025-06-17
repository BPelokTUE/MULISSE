#include "Index/EnvelopeIndex/EnvelopeIndex.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Entry/SaxEnvelope.hpp"
#include "Index/Sax/SaxWord.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

// Envelope

template <>
EnvelopeIndex<Envelope>::EnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                       const uint pos_per_env)
    : m_ch_segmentation_strategy(ch_segmentation_strategy), m_pos_per_env(pos_per_env) {}

template <>
void EnvelopeIndex<Envelope>::insert(IndexEntry<Envelope> &entry) {
    m_entries.push_back(entry);
}

template <>
auto EnvelopeIndex<Envelope>::get_entry(uint ind) const -> decltype(auto) {
    // Return a reference to the entry at the given index
    return m_entries.at(ind);
}

template <>
const uint EnvelopeIndex<Envelope>::size() const {
    return U(m_entries.size());
}

// SaxEnvelope

template <>
EnvelopeIndex<SaxEnvelope>::EnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                          const uint pos_per_env)
    : m_ch_segmentation_strategy(ch_segmentation_strategy), m_pos_per_env(pos_per_env) {
    m_breakpoints = &RunSettings::get_instance().get_breakpoints();
}

template <>
void EnvelopeIndex<SaxEnvelope>::insert(IndexEntry<Envelope> &entry) {
    auto &RS = RunSettings::get_instance();
    const auto &breakpoints = RS.get_breakpoints();
    SaxNumBitsT num_bits = RS.get_breakpoint_props().m_breakpoint_num_bits;

    vec<SaxEnvelope> sax_envelopes;
    sax_envelopes.reserve(entry.m_mts_summary.size());
    for (const auto &env : entry.m_mts_summary) sax_envelopes.emplace_back(env, breakpoints, num_bits);

    m_entries.push_back(IndexEntry<SaxEnvelope>{entry.m_subs_info, sax_envelopes});
}

template <>
auto EnvelopeIndex<SaxEnvelope>::get_entry(uint ind) const -> decltype(auto) {
    // Return a new IndexEntry<Envelope> generated from the SaxEnvelope at the given index
    vec<Envelope> envelopes;
    envelopes.reserve(m_entries.at(ind).m_mts_summary.size());
    for (const auto &sax_env : m_entries.at(ind).m_mts_summary)
        envelopes.push_back(sax_env.to_envelope(*m_breakpoints));

    return IndexEntry<Envelope>{.m_subs_info = m_entries.at(ind).m_subs_info, .m_mts_summary = envelopes};
}

template <>
const uint EnvelopeIndex<SaxEnvelope>::size() const {
    return U(m_entries.size());
}
