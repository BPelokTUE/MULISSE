#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif

#include "Index/EnvelopeIndex/EnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Index/Sax/InvSax.hpp"
#include "Util/HelperFuncs/Math.hpp"

InvSaxSortingEnvelopeGrouper::InvSaxSortingEnvelopeGrouper(SaxNumBitsT num_bits) : m_num_bits(num_bits) {}

void InvSaxSortingEnvelopeGrouper::sort_envelope_entries(vec<IndexEntry<Envelope>> &entries) {
    // 1. Calculate invSAX
    vec<std::pair<InvSax<Envelope>, int>> inv_sax_entries(entries.size());

    OMP_PRAGMA(omp parallel for)
    for (size_t i = 0; i < entries.size(); ++i)
        inv_sax_entries[i] = {InvSax<Envelope>(entries[i].m_mts_summary, m_num_bits), static_cast<int>(i)};

    // 2. Sort entries based on invSAX

#ifdef DISABLE_PARALLELISM
    std::sort(inv_sax_entries.begin(), inv_sax_entries.end());
#else
    tbb::parallel_sort(inv_sax_entries.begin(), inv_sax_entries.end());
#endif

    // 2.1. Apply ordering to entries
    for (size_t i = 0; i < entries.size(); ++i) {
        size_t j = i;
        // Until the permutation cycle is resolved, keep iterating
        while (inv_sax_entries[j].second >= 0) {
            int &loc = inv_sax_entries[j].second;  // Get the location of the entry
            j = static_cast<size_t>(loc);          // Update the index
            std::swap(entries[i], entries[j]);     // Swap the entries
            loc = -1;                              // Mark the location as resolved
        }
    }
}

// BucketingEnvelopeGrouper

BucketingEnvelopeGrouper::BucketingEnvelopeGrouper(size_t bucket_size) : m_bucket_size(bucket_size) {}

vec<uptr<EnvelopeNode>> BucketingEnvelopeGrouper::group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) {
    size_t bucket_size = std::min(static_cast<size_t>(m_bucket_size), envelope_entries.size());
    size_t num_buckets = (envelope_entries.size() + bucket_size - 1) / bucket_size;
    vec<vec<uptr<EnvelopeNode>>> buckets(num_buckets), act_layer_nodes;

    OMP_PRAGMA(omp parallel for)
    for (size_t b_ind = 0; b_ind < num_buckets; ++b_ind) {
        size_t act_bucket_size = get_bucket_size(b_ind, num_buckets, envelope_entries.size());
        buckets[b_ind].resize(act_bucket_size);
        for (size_t i = 0; i < act_bucket_size; ++i) {
            size_t entry_ind = bucket_size * b_ind + i;
            buckets[b_ind][i] = std::make_unique<EnvelopeLeaf>(envelope_entries[entry_ind]);
        }
    }

    envelope_entries.resize(0);

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

// InvSaxSortingBucketingEnvelopeGrouper

vec<uptr<EnvelopeNode>> InvSaxSortingBucketingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>> &envelope_entries) {
    sort_envelope_entries(envelope_entries);
    return BucketingEnvelopeGrouper::group_envelope_entries(envelope_entries);
}
