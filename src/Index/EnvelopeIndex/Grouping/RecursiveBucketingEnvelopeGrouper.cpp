#include "Index/EnvelopeIndex/Grouping/RecursiveBucketingEnvelopeGrouper.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

RecursiveBucketingEnvelopeGrouper::RecursiveBucketingEnvelopeGrouper(size_t bucket_size)
    : BucketingEnvelopeGrouper(bucket_size) {}

vec<uptr<EnvelopeNode>> RecursiveBucketingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>>::iterator entries_begin, vec<IndexEntry<Envelope>>::iterator entries_end) {
    size_t num_entries = static_cast<size_t>(entries_end - entries_begin);
    size_t bucket_size = std::min(static_cast<size_t>(m_bucket_size), num_entries);
    size_t num_buckets = (num_entries + bucket_size - 1) / bucket_size;
    vec<vec<uptr<EnvelopeNode>>> buckets(num_buckets), act_layer_nodes;

    OMP_PRAGMA(omp parallel for)
    for (size_t b_ind = 0; b_ind < num_buckets; ++b_ind) {
        size_t act_bucket_size = get_bucket_size(b_ind, num_buckets, num_entries);
        buckets[b_ind].resize(act_bucket_size);
        for (size_t i = 0; i < act_bucket_size; ++i) {
            int entry_ind = static_cast<int>(bucket_size * b_ind + i);
            buckets[b_ind][i] = std::make_unique<EnvelopeLeaf>(*(entries_begin + entry_ind));
        }
    }

    // Create hierarchy
    do {
        size_t layer_num_buckets = (num_buckets + bucket_size - 1) / bucket_size;
        act_layer_nodes = vec<vec<uptr<EnvelopeNode>>>(layer_num_buckets);

        OMP_PRAGMA(omp parallel for)
        for (size_t layer_b_ind = 0; layer_b_ind < layer_num_buckets; ++layer_b_ind) {
            size_t act_bucket_size = get_bucket_size(layer_b_ind, layer_num_buckets, num_buckets);
            act_layer_nodes[layer_b_ind].resize(act_bucket_size);
            for (size_t i = 0; i < act_bucket_size; ++i) {
                size_t b_ind = bucket_size * layer_b_ind + i;
                act_layer_nodes[layer_b_ind][i] = std::make_unique<EnvelopeInternal>(std::move(buckets[b_ind]));
            }
        }
        buckets = std::move(act_layer_nodes);
        num_buckets = layer_num_buckets;
    } while (num_buckets > 1);

    return std::move(buckets[0]);
}
