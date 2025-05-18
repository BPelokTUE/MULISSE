#include "Index/EnvelopeIndex/Flat/FlatEnvelopeIndex.hpp"

#include "Enums/EntryInserterType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Index/Sax/SaxWord.hpp"
#include "Serialization/Macros.hpp"
#include "Util/RunSettings/RunSettings.hpp"

FlatEnvelopeIndex::FlatEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, uint pos_per_env,
                                     SaxNumBitsT sax_num_bits)
    : EnvelopeIndex(ch_segmentation_strategy, pos_per_env), m_sax_num_bits(sax_num_bits) {}

void FlatEnvelopeIndex::insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) {
    uptr<IEntryInserter<FlatEnvelopeIndex>> inserter;
    switch (inserter_type) {
        case PARALLEL:  // Temporary solution to support two-stage indexes
        case TOP_DOWN:
            inserter = std::make_unique<TopDownInserter<FlatEnvelopeIndex>>(this->shared_from_this());
            break;
        default:
            throw std::invalid_argument("Invalid inserter type");
    }
    inserter->insert_entries(entries);
};

void FlatEnvelopeIndex::insert(IndexEntry<Envelope> &entry) {
    if (m_sax_num_bits > 0) {
        auto &breakpoints = RunSettings::get_instance().get_breakpoints();
        SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(entry.m_mts_summary[0].m_lower.size());

        for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
            SaxWord sax_lower(entry.m_mts_summary[c].m_lower, breakpoints, m_sax_num_bits);
            SaxWord sax_upper(entry.m_mts_summary[c].m_upper, breakpoints, m_sax_num_bits);

            for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) {
                entry.m_mts_summary[c].m_lower[s] = sax_lower[s] > 0 ? breakpoints[sax_lower[s] - 1] : -INF;
                entry.m_mts_summary[c].m_upper[s] = sax_upper[s] < breakpoints.size() ? breakpoints[sax_upper[s]] : INF;
            }
        }
    }
    EnvelopeIndex::insert(entry);
}

uptr<IFinalizedIndex<EnvelopeTag>> FlatEnvelopeIndex::finalize() {
    return std::make_unique<FinalizedFlatEnvelopeIndex>(m_ch_segmentation_strategy, m_pos_per_env,
                                                        std::move(m_entries));
}
