#include "Index/EnvelopeIndex/Grouping/VarianceLimitingEnvelopeGrouper.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Util/Constants/Math.hpp"

VarianceLimitingEnvelopeGrouper::VarianceLimitingEnvelopeGrouper(Real max_width_change)
    : m_max_width_change(max_width_change) {}

vec<uptr<EnvelopeNode>> VarianceLimitingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>>::iterator entries_begin, vec<IndexEntry<Envelope>>::iterator entries_end) {
    // Iterate over all entries, keep track of the minimum lower and maximum upper for each segment, if the update to
    // the average width is less than or equal to m_max_width_change, the current entry is merged into the current node,
    // otherwise a new node is created.
    if (entries_begin == entries_end) return {};

    vec<uptr<EnvelopeNode>> nodes;
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(entries_begin->m_mts_summary.size());
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(entries_begin->m_mts_summary[0].size());

    vec<Envelope> merged_envelopes(entries_begin->m_mts_summary), node_envelopes(entries_begin->m_mts_summary);
    vec<SubsequenceInfo> node_subs_infos(1, entries_begin->m_subs_info);

    int num_entries = static_cast<int>(entries_end - entries_begin);
    for (int i = 1; i < num_entries; ++i) {
        auto &entry = *(entries_begin + i);
        Real new_width = 0.0, old_width = 0.0;
        for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
            for (SaxSegIndT s = 0; s < entry.m_mts_summary[c].size(); ++s) {
                old_width += merged_envelopes[c].m_upper[s] - merged_envelopes[c].m_lower[s];

                merged_envelopes[c].m_lower[s] =
                    std::min(entry.m_mts_summary[c].m_lower[s], merged_envelopes[c].m_lower[s]);
                merged_envelopes[c].m_upper[s] =
                    std::max(entry.m_mts_summary[c].m_upper[s], merged_envelopes[c].m_upper[s]);

                new_width += merged_envelopes[c].m_upper[s] - merged_envelopes[c].m_lower[s];
            }
        }
        Real width_update = (new_width - old_width) / old_width;
        width_update /= num_channels * num_segments;
        if (width_update <= m_max_width_change) {
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) node_envelopes[c].merge(entry.m_mts_summary[c]);
            node_subs_infos.push_back(entry.m_subs_info);
        } else {
            nodes.push_back(std::make_unique<EnvelopeLeaf>(std::move(node_envelopes), std::move(node_subs_infos)));

            merged_envelopes = entry.m_mts_summary;
            node_envelopes = entry.m_mts_summary;
            node_subs_infos = {entry.m_subs_info};
        }
    }
    nodes.push_back(std::make_unique<EnvelopeLeaf>(node_envelopes, node_subs_infos));

    return nodes;
}
