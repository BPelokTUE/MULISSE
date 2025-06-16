#include "Index/EnvelopeIndex/Flat/FlatEnvelopeIndex.hpp"

#include "Enums/EntryInserterType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Entry/SaxEnvelope.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Index/Sax/SaxWord.hpp"
#include "Serialization/Macros.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <typename EnvT>
FlatEnvelopeIndex<EnvT>::FlatEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                           uint pos_per_env)
    : EnvelopeIndex<EnvT>(ch_segmentation_strategy, pos_per_env) {}

template <typename EnvT>
void FlatEnvelopeIndex<EnvT>::insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) {
    uptr<IEntryInserter<FlatEnvelopeIndex<EnvT>>> inserter;
    switch (inserter_type) {
        case PARALLEL:  // Temporary solution to support two-stage indexes
        case TOP_DOWN:
            inserter = std::make_unique<TopDownInserter<FlatEnvelopeIndex<EnvT>>>(this->shared_from_this());
            break;
        default:
            throw std::invalid_argument("Invalid inserter type");
    }
    inserter->insert_entries(entries);
};

template <typename EnvT>
void FlatEnvelopeIndex<EnvT>::insert(IndexEntry<Envelope> &entry) {
    EnvelopeIndex<EnvT>::insert(entry);
}

template <typename EnvT>
uptr<IFinalizedIndex<EnvelopeTag>> FlatEnvelopeIndex<EnvT>::finalize() {
    return std::make_unique<FinalizedFlatEnvelopeIndex<EnvT>>(this->m_ch_segmentation_strategy, this->m_pos_per_env,
                                                              std::move(this->m_entries));
}

// Explicit instantiation for Envelope and SaxEnvelope
template class FlatEnvelopeIndex<Envelope>;
template class FlatEnvelopeIndex<SaxEnvelope>;
