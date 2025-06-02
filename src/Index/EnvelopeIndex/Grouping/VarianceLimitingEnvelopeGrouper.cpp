#include "Index/EnvelopeIndex/Grouping/VarianceLimitingEnvelopeGrouper.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Util/Constants/Math.hpp"

VarianceLimitingEnvelopeGrouper::VarianceLimitingEnvelopeGrouper(Real max_width_change)
    : m_max_width_change(max_width_change) {}

vec<uptr<EnvelopeNode>> VarianceLimitingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>> &envelope_entries) {
    // Iterate over all entries, keep track of the maximum lower and minimum upper for each segment, if the update to
    // the average width is less than or equal to m_max_width_change, the current entry is merged into the current node,
    // otherwise a new node is created.
    if (envelope_entries.empty()) return {};

    vec<uptr<EnvelopeNode>> nodes;
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(envelope_entries[0].m_mts_summary.size());
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(envelope_entries[0].m_mts_summary[0].size());

    vec<Envelope> minimal_envelopes(envelope_entries[0].m_mts_summary),
        node_envelopes(envelope_entries[0].m_mts_summary);
    vec<SubsequenceInfo> node_subs_infos(1, envelope_entries[0].m_subs_info);

    for (size_t i = 1; i < envelope_entries.size(); ++i) {
        auto &entry = envelope_entries[i];
        Real width_update = 0.0;
        for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
            for (SaxSegIndT s = 0; s < entry.m_mts_summary[c].size(); ++s) {
                minimal_envelopes[c].m_lower[s] =
                    std::max(entry.m_mts_summary[c].m_lower[s], minimal_envelopes[c].m_lower[s]);
                minimal_envelopes[c].m_upper[s] =
                    std::min(entry.m_mts_summary[c].m_upper[s], minimal_envelopes[c].m_upper[s]);

                width_update += std::abs(minimal_envelopes[c].m_upper[s] - minimal_envelopes[c].m_lower[s] -
                                         (entry.m_mts_summary[c].m_upper[s] - entry.m_mts_summary[c].m_lower[s]));
            }
        }
        width_update /= num_channels * num_segments;
        if (width_update <= m_max_width_change) {
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) node_envelopes[c].merge(entry.m_mts_summary[c]);
            node_subs_infos.push_back(entry.m_subs_info);
        } else {
            nodes.push_back(std::make_unique<EnvelopeLeaf>(std::move(node_envelopes), std::move(node_subs_infos)));

            minimal_envelopes = entry.m_mts_summary;
            node_envelopes = entry.m_mts_summary;
            node_subs_infos = {entry.m_subs_info};
        }
    }
    nodes.push_back(std::make_unique<EnvelopeLeaf>(node_envelopes, node_subs_infos));

    return nodes;
}
