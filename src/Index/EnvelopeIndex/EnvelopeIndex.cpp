#include "Index/EnvelopeIndex/EnvelopeIndex.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

EnvelopeIndex::EnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env)
    : m_ch_segmentation_strategy(ch_segmentation_strategy), m_pos_per_env(pos_per_env) {}

void EnvelopeIndex::insert(IndexEntry<Envelope> &entry) { m_entries.push_back(entry); }

const vec<IndexEntry<Envelope>> &EnvelopeIndex::get_entries() const { return m_entries; }
