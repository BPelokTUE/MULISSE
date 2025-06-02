#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"

FinalizedFlatEnvelopeIndex::FinalizedFlatEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                                       const uint pos_per_env, vec<IndexEntry<Envelope>> &&entries)
    : FinalizedEnvelopeIndex(ch_segmentation_strategy, pos_per_env), m_entries(std::move(entries)) {}

const vec<IndexEntry<Envelope>> &FinalizedFlatEnvelopeIndex::get_entries() const { return m_entries; }
