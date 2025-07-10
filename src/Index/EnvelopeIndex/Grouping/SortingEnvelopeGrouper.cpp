#include "Index/EnvelopeIndex/Grouping/SortingEnvelopeGrouper.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Sax/InvSax.hpp"
#include "Util/HelperFuncs/ParallelSort.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

SortingEnvelopeGrouper::SortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper)
    : m_extra_grouper(std::move(extra_grouper)) {}

vec<uptr<EnvelopeNode>> SortingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>>::iterator entries_begin, vec<IndexEntry<Envelope>>::iterator entries_end) {
    parallel_sort(entries_begin, entries_end);
    return m_extra_grouper->group_envelope_entries(entries_begin, entries_end);
}
