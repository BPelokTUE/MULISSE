#ifndef TREE_ENVELOPE_INDEX_HPP
#define TREE_ENVELOPE_INDEX_HPP

#include "Index/EnvelopeIndex/EnvelopeIndex.hpp"

class IEnvelopeGrouper;
class EnvelopeNode;

class TreeEnvelopeIndex : public EnvelopeIndex, public std::enable_shared_from_this<TreeEnvelopeIndex> {
   public:
    /**
     * @brief Construct a new TreeEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env Number of positions per envelope
     * @param grouper The EnvelopeGrouper to use for grouping envelope entries
     */
    TreeEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env,
                      uptr<IEnvelopeGrouper> grouper);

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override;

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override;

   private:
    uptr<IEnvelopeGrouper> m_grouper;
};

#endif  // ENVELOPE_INDEX_HPP
