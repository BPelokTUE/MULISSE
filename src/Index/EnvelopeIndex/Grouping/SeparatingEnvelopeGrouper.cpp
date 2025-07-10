#include "Index/EnvelopeIndex/Grouping/SeparatingEnvelopeGrouper.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/ParallelSort.hpp"

SeparatingEnvelopeGrouper::SeparatingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper)
    : m_extra_grouper(std::move(extra_grouper)) {}

vec<uptr<EnvelopeNode>> SeparatingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>>::iterator entries_begin, vec<IndexEntry<Envelope>>::iterator entries_end) {
    parallel_sort(entries_begin, entries_end);

    vec<uptr<EnvelopeNode>> grouped_entries;
    auto series_begin = entries_begin, series_end = entries_begin;
    uint series = 0;
    while (series_begin != entries_end) {
        while (series_end != entries_end && series_begin->m_subs_info.m_position.m_series == series) ++series_end;

        if (series_begin != series_end) {
            auto series_grouped_entries = m_extra_grouper->group_envelope_entries(series_begin, series_end);
            grouped_entries.insert(grouped_entries.end(), std::make_move_iterator(series_grouped_entries.begin()),
                                   std::make_move_iterator(series_grouped_entries.end()));
        }
        ++series;
    }
    return grouped_entries;
}
