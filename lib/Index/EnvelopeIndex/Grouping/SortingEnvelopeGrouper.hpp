#ifndef INDEX_ENVELOPEINDEX_GROUPING_SORTINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_SORTINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

/** @brief Envelope grouper that sorts envelopes, intended as a first step in the grouping process */
class SortingEnvelopeGrouper : public IEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new InvSaxSortingEnvelopeGrouper instance
     * @param extra_grouper The IEnvelopeGrouper to use after sorting
     */
    SortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;

   private:
    uptr<IEnvelopeGrouper> m_extra_grouper;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_SORTINGENVELOPEGROUPER_HPP
