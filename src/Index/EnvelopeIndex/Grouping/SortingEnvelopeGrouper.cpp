#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif

#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/Grouping/SortingEnvelopeGrouper.hpp"
#include "Index/Sax/InvSax.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

SortingEnvelopeGrouper::SortingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper)
    : m_extra_grouper(std::move(extra_grouper)) {}

vec<uptr<EnvelopeNode>> SortingEnvelopeGrouper::group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) {
#ifdef DISABLE_PARALLELISM
    std::sort(envelope_entries.begin(), envelope_entries.end());
#else
    tbb::parallel_sort(envelope_entries.begin(), envelope_entries.end());
#endif
    return m_extra_grouper->group_envelope_entries(envelope_entries);
}
