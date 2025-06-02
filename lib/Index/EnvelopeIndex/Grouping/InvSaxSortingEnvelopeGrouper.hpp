#ifndef INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/BucketingEnvelopeGrouper.hpp"

/** @brief Abstract class for enveloper groupers that use invSAX-based sorting */
class InvSaxSortingEnvelopeGrouper : virtual public IEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new InvSaxSortingEnvelopeGrouper instance
     * @param num_bits Number of bits for the iSAX representation
     */
    InvSaxSortingEnvelopeGrouper(SaxNumBitsT num_bits);

   protected:
    /**
     * @brief Sort envelope_entries in-place using their invSAX representation
     * @param envelope_entries Vector of envelope_entries to sort
     */
    void sort_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries);

   private:
    SaxNumBitsT m_num_bits;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGENVELOPEGROUPER_HPP
