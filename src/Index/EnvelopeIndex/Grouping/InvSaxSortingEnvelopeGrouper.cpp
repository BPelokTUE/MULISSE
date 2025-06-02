#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif

#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/Grouping/InvSaxSortingEnvelopeGrouper.hpp"
#include "Index/Sax/InvSax.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

InvSaxSortingEnvelopeGrouper::InvSaxSortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper, SaxNumBitsT num_bits)
    : m_extra_grouper(std::move(extra_grouper)), m_num_bits(num_bits) {}

vec<uptr<EnvelopeNode>> InvSaxSortingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>> &envelope_entries) {
    // 1. Calculate invSAX
    vec<std::pair<InvSax<Envelope>, int>> inv_sax_entries(envelope_entries.size());

    OMP_PRAGMA(omp parallel for)
    for (size_t i = 0; i < envelope_entries.size(); ++i)
        inv_sax_entries[i] = {InvSax<Envelope>(envelope_entries[i].m_mts_summary, m_num_bits), static_cast<int>(i)};

    // 2. Sort entries based on invSAX

#ifdef DISABLE_PARALLELISM
    std::sort(inv_sax_entries.begin(), inv_sax_entries.end());
#else
    tbb::parallel_sort(inv_sax_entries.begin(), inv_sax_entries.end());
#endif

    // 2.1. Apply ordering to entries
    for (size_t i = 0; i < envelope_entries.size(); ++i) {
        size_t j = i;
        // Until the permutation cycle is resolved, keep iterating
        while (inv_sax_entries[j].second >= 0) {
            int &loc = inv_sax_entries[j].second;                 // Get the location of the entry
            j = static_cast<size_t>(loc);                         // Update the index
            std::swap(envelope_entries[i], envelope_entries[j]);  // Swap the entries
            loc = -1;                                             // Mark the location as resolved
        }
    }
    // 3. Apply extra grouping
    return m_extra_grouper->group_envelope_entries(envelope_entries);
}
