#include "Index/EnvelopeIndex/Tree/TreeEnvelopeIndex.hpp"

#include <algorithm>

#include "Util/Types/Pointers.hpp"
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif

#include "Enums/EntryInserterType.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryInserter/EntryInserter.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/EnvelopeIndex/EnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Tree/FinalizedTreeEnvelopeIndex.hpp"

TreeEnvelopeIndex::TreeEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                     const uint pos_per_env, uptr<IEnvelopeGrouper> grouper)
    : EnvelopeIndex(ch_segmentation_strategy, pos_per_env), m_grouper(std::move(grouper)) {}

void TreeEnvelopeIndex::insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) {
    uptr<IEntryInserter<TreeEnvelopeIndex>> inserter;
    switch (inserter_type) {
        case PARALLEL:  // Temporary solution to support two-stage indexes
        case TOP_DOWN:
            inserter = std::make_unique<TopDownInserter<TreeEnvelopeIndex>>(this->shared_from_this());
            break;
        default:
            throw std::invalid_argument("Invalid inserter type");
    }
    inserter->insert_entries(entries);
};

uptr<IFinalizedIndex<EnvelopeTag>> TreeEnvelopeIndex::finalize() {
    // 1. Group entries
    m_first_layer_nodes = m_grouper->group_envelope_entries(m_entries);

    // 2. Merge entries in leaves of the tree

    // 3. Return the finalized index (this)
    return std::make_unique<FinalizedTreeEnvelopeIndex>(m_ch_segmentation_strategy, m_pos_per_env,
                                                        std::move(m_first_layer_nodes));
}
