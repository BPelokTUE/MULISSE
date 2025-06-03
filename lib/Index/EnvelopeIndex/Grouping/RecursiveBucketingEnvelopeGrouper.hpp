#ifndef INDEX_ENVELOPEINDEX_GROUPING_RECURSIVEBUCKETINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_RECURSIVEBUCKETINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/BucketingEnvelopeGrouper.hpp"

/** @brief Grouper that creates a balanced index by recursively grouping together buckets of entries */
class RecursiveBucketingEnvelopeGrouper : public BucketingEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new RecursiveBucketingEnvelopeGrouper instance
     * @param bucket_size Size of the buckets
     */
    RecursiveBucketingEnvelopeGrouper(size_t bucket_size);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_RECURSIVEBUCKETINGENVELOPEGROUPER_HPP
