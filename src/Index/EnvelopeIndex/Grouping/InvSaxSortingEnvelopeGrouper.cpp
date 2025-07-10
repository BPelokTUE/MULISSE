#include "Index/EnvelopeIndex/Grouping/InvSaxSortingEnvelopeGrouper.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Sax/InvSax.hpp"
#include "Util/HelperFuncs/ParallelSort.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

InvSaxSortingEnvelopeGrouper::InvSaxSortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper, SaxNumBitsT num_bits)
    : m_extra_grouper(std::move(extra_grouper)), m_num_bits(num_bits) {}

vec<uptr<EnvelopeNode>> InvSaxSortingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>>::iterator entries_begin, vec<IndexEntry<Envelope>>::iterator entries_end) {
    int num_entries = static_cast<int>(entries_end - entries_begin);
    // 1. Calculate invSAX
    vec<std::pair<InvSax<Envelope>, int>> inv_sax_entries(U(num_entries));

    OMP_PRAGMA(omp parallel for)
    for (int i = 0; i < num_entries; ++i)
        inv_sax_entries[U(i)] = {InvSax<Envelope>((entries_begin + i)->m_mts_summary, m_num_bits), i};

    // 2. Sort entries based on invSAX
    parallel_sort(inv_sax_entries.begin(), inv_sax_entries.end());

    // 2.1. Apply ordering to entries
    for (int i = 0; i < num_entries; ++i) {
        int j = i;
        // Until the permutation cycle is resolved, keep iterating
        while (inv_sax_entries[U(j)].second >= 0) {
            int &loc = inv_sax_entries[U(j)].second;               // Get the location of the entry
            j = loc;                                               // Update the index
            std::iter_swap(entries_begin + i, entries_begin + j);  // Swap the entries
            loc = -1;                                              // Mark the location as resolved
        }
    }
    // 3. Apply extra grouping
    return m_extra_grouper->group_envelope_entries(entries_begin, entries_end);
}
