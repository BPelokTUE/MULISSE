#include "Index/EnvelopeIndex/Grouping/BucketingEnvelopeGrouper.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

BucketingEnvelopeGrouper::BucketingEnvelopeGrouper(size_t bucket_size) : m_bucket_size(bucket_size) {}

size_t BucketingEnvelopeGrouper::get_bucket_size(const size_t b_ind, const size_t num_buckets,
                                                 const size_t num_items) const {
    if (b_ind == num_buckets - 1) {
        size_t remainder = num_items % m_bucket_size;
        return remainder == 0 ? m_bucket_size : remainder;
    }
    return m_bucket_size;
}

vec<uptr<EnvelopeNode>> BucketingEnvelopeGrouper::group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) {
    size_t bucket_size = std::min(static_cast<size_t>(m_bucket_size), envelope_entries.size());
    size_t num_buckets = (envelope_entries.size() + bucket_size - 1) / bucket_size;

    vec<uptr<EnvelopeNode>> buckets(num_buckets);

    OMP_PRAGMA(omp parallel for)
    for (size_t b_ind = 0; b_ind < num_buckets; ++b_ind) {
        size_t act_bucket_size = get_bucket_size(b_ind, num_buckets, envelope_entries.size());

        size_t bucket_start_ind = b_ind * bucket_size;
        vec<Envelope> bucket_mts_envelope(envelope_entries[bucket_start_ind].m_mts_summary);
        vec<SubsequenceInfo> bucket_subs_infos({envelope_entries[bucket_start_ind].m_subs_info});

        for (size_t i = 1; i < act_bucket_size; ++i) {
            size_t entry_ind = bucket_start_ind + i;
            const auto &entry = envelope_entries[entry_ind];

            for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c)
                bucket_mts_envelope[c].merge(entry.m_mts_summary[c]);
            bucket_subs_infos.push_back(entry.m_subs_info);
        }
        buckets[b_ind] = std::make_unique<EnvelopeLeaf>(std::move(bucket_mts_envelope), std::move(bucket_subs_infos));
    }

    return std::move(buckets);
}
