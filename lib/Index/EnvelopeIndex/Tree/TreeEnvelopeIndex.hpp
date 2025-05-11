#ifndef TREE_ENVELOPE_INDEX_HPP
#define TREE_ENVELOPE_INDEX_HPP

#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif
#include <queue>

#include "Enums/EntryInserterType.hpp"
#include "Index/EntryInserter/EntryInserter.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/EnvelopeIndex/EnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/EnvelopeIndex.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Index/EnvelopeIndex/Tree/FinalizedTreeEnvelopeIndex.hpp"
#include "Serialization/Macros.hpp"

class TreeEnvelopeIndex : public EnvelopeIndex, public std::enable_shared_from_this<TreeEnvelopeIndex> {
   public:
    /**
     * @brief Construct a new TreeEnvelopeIndex instance
     * @param segmentation_strategy The segmentation strategy to use
     * @param pos_per_env Number of positions per envelope
     * @param grouper The EnvelopeGrouper to use for grouping envelope entries
     */
    TreeEnvelopeIndex(sptr<ISegmentationStrategy> segmentation_strategy, const uint pos_per_env,
                      uptr<IEnvelopeGrouper> grouper)
        : EnvelopeIndex(segmentation_strategy, pos_per_env), m_grouper(std::move(grouper)) {}

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override {
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

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override {
        // 1. Group entries
        m_first_layer_nodes = m_grouper->group_envelope_entries(m_entries);

        // 2. Merge entries in leaves of the tree

        // 3. Return the finalized index (this)
        return std::make_unique<FinalizedTreeEnvelopeIndex>(m_segmentation_strategy, m_pos_per_env,
                                                            std::move(m_first_layer_nodes));
    }

   private:
    uptr<IEnvelopeGrouper> m_grouper;
    vec<uptr<EnvelopeNode>> m_first_layer_nodes;
};

#endif  // ENVELOPE_INDEX_HPP
