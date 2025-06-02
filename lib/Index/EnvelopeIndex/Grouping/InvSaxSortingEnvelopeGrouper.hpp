#ifndef INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

/** @brief Abstract class for enveloper groupers that use invSAX-based sorting */
class InvSaxSortingEnvelopeGrouper : public IEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new InvSaxSortingEnvelopeGrouper instance
     * @param grouper The IEnvelopeGrouper to use after sorting
     * @param num_bits Number of bits for the iSAX representation
     */
    InvSaxSortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper, SaxNumBitsT num_bits);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;

   private:
    uptr<IEnvelopeGrouper> m_extra_grouper;
    SaxNumBitsT m_num_bits;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP
