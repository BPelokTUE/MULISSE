#ifndef INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGBUCKETINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGBUCKETINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/BucketingEnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Grouping/InvSaxSortingEnvelopeGrouper.hpp"

/** @brief Combination of BucketingEnvelopeGrouper and InvSaxSortingEnvelopeGrouper */
class InvSaxSortingBucketingEnvelopeGrouper : virtual public InvSaxSortingEnvelopeGrouper,
                                              virtual public BucketingEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new InvSaxSortingBucketingEnvelopeGrouper instance
     * @param num_bits Number of bits for the iSAX representation
     * @param bucket_size Size of the buckets
     */
    InvSaxSortingBucketingEnvelopeGrouper(SaxNumBitsT num_bits, size_t bucket_size)
        : InvSaxSortingEnvelopeGrouper(num_bits), BucketingEnvelopeGrouper(bucket_size) {}

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_INVSAXSORTINGBUCKETINGENVELOPEGROUPER_HPP
