#ifndef ENVELOPE_GROUPER_HPP
#define ENVELOPE_GROUPER_HPP

#include "Util/typedefs.hpp"
#include "Search/Envelope/EnvelopeNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/IndexEntry.hpp"

/** @brief Interface for envelope groupers */
class IEnvelopeGrouper {
   public:
    virtual ~IEnvelopeGrouper() = default;

    /**
     * @brief Group envelope_entries into a tree structure
     * @param envelope_entries Vector of envelope_entries to group
     * @return Vector of first layer envelope nodes
     */
    virtual vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) = 0;
};

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

    inline size_t get_bucket_size(const size_t b_ind, const size_t num_buckets, const size_t num_items) const {
        if (b_ind == num_buckets - 1) {
            size_t remainder = num_items % m_bucket_size;
            return remainder == 0 ? m_bucket_size : remainder;
        }
        return m_bucket_size;
    }
};

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

#endif  // ENVELOPE_GROUPER_HPP
