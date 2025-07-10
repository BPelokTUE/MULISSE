#ifndef INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

/** @brief IEnvelopeGrouper that uses invSAX-based sorting, intended as a first step in the grouping process */
class InvSaxSortingEnvelopeGrouper : public IEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new InvSaxSortingEnvelopeGrouper instance
     * @param extra_grouper The IEnvelopeGrouper to use after sorting
     * @param num_bits Number of bits for the iSAX representation
     */
    InvSaxSortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper, SaxNumBitsT num_bits);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>>::iterator entries_begin,
                                                   vec<IndexEntry<Envelope>>::iterator entries_end) override;

   private:
    uptr<IEnvelopeGrouper> m_extra_grouper;
    SaxNumBitsT m_num_bits;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP
