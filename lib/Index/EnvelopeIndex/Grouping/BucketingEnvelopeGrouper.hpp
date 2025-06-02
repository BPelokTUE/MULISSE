#ifndef INDEX_ENVELOPEINDEX_GROUPING_BUCKETINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_BUCKETINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

/** @brief Grouper that creates a balanced index by recursively grouping together buckets of entries */
class BucketingEnvelopeGrouper : virtual public IEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new BucketingEnvelopeGrouper instance
     * @param bucket_size Size of the buckets
     */
    BucketingEnvelopeGrouper(size_t bucket_size);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;

   private:
    size_t m_bucket_size;

    size_t get_bucket_size(const size_t b_ind, const size_t num_buckets, const size_t num_items) const;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_BUCKETINGENVELOPEGROUPER_HPP
