#include "Modules/IndexStats.hpp"

#include "Index/ChainIndex/FinalizedChainIndex.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Index/Index.hpp"
#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Util/Logging/IndexStatsLogger.hpp"

// iSAX

template <>
void IndexAnalyzer<FinalizedISaxIndex<PaaTag>, PaaTag>::analyze(uint length_group_id, bool separate_segment_stats) {
    analyze_isax(length_group_id, separate_segment_stats);
}

// iSAX + envelope

template <>
void IndexAnalyzer<FinalizedISaxIndex<EnvelopeTag>, EnvelopeTag>::analyze(uint length_group_id,
                                                                          bool separate_segment_stats) {
    analyze_isax(length_group_id, separate_segment_stats);
}

// Envelope / SAX envelope

template <>
void IndexAnalyzer<FinalizedFlatEnvelopeIndex, EnvelopeTag>::analyze(uint length_group_id,
                                                                     bool separate_segment_stats) {
    const FinalizedFlatEnvelopeIndex *index = dynamic_cast<FinalizedFlatEnvelopeIndex *>(m_index.get());
    if (!index) throw std::runtime_error("Could not cast index to FinalizedFlatEnvelopeIndex");

    const auto &entries = index->get_entries();
    if (entries.empty()) {
        throw std::runtime_error("The list of entries is empty");
    }
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(entries[0].m_mts_summary.size());
    if (num_channels == 0) {
        throw std::runtime_error("Number of channels is 0");
    }
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(entries[0].m_mts_summary[0].size());
    if (num_segments == 0) {
        std::cout << "Warning: Number of segments is 0\n";
    }
    IndexStats stats(num_channels, num_segments, separate_segment_stats);

    for (const IndexEntry<Envelope> &entry : index->get_entries()) {
        stats.update_leaf_stats(1, 1);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto &summary = entry.m_mts_summary[c];
            for (SaxSegIndT s = 0; s < summary.size(); ++s) {
                Real lower = summary.m_lower[s], upper = summary.m_upper[s];
                stats.update_seg_stats(lower, upper, c, s);
            }
        }
    }
    stats.calculate();
    IndexStatsLogger::write_entry(stats, length_group_id, m_sub_index_id, separate_segment_stats);
}

// iSAX + envelope / SAX envelope

template <>
uptr<FinalizedChainIndex<EnvelopeTag>> IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag>::create_index() {
    vec<uptr<IFinalizedIndex<EnvelopeTag>>> approx_indexes(1);
    approx_indexes[0] = std::make_unique<FinalizedISaxIndex<EnvelopeTag>>();
    auto exact_index = uptr<IFinalizedIndex<EnvelopeTag>>(new FinalizedFlatEnvelopeIndex());
    return std::make_unique<FinalizedChainIndex<EnvelopeTag>>(std::move(approx_indexes), std::move(exact_index));
}

template <>
void IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag>::analyze(uint length_group_id,
                                                                           bool separate_segment_stats) {
    auto approx_index = uptr<FinalizedISaxIndex<EnvelopeTag>>(
        static_cast<FinalizedISaxIndex<EnvelopeTag> *>(m_index->release_approx_index(0)));
    IndexAnalyzer<FinalizedISaxIndex<EnvelopeTag>, EnvelopeTag>(std::move(approx_index), 0u)
        .analyze(length_group_id, separate_segment_stats);

    auto exact_index =
        uptr<FinalizedFlatEnvelopeIndex>(static_cast<FinalizedFlatEnvelopeIndex *>(m_index->release_exact_index()));
    IndexAnalyzer<FinalizedFlatEnvelopeIndex, EnvelopeTag>(std::move(exact_index), 1u)
        .analyze(length_group_id, separate_segment_stats);
}

// Main

int calculate_index_stats(SearchMethodType method_type, uint num_l_groups, ArchiveType index_format,
                          bool separate_segment_stats) {
    switch (method_type) {
        case ISAX: {
            IndexAnalyzer<FinalizedISaxIndex<PaaTag>, PaaTag>::analyze_run_index(index_format, num_l_groups,
                                                                                 separate_segment_stats);
            break;
        }
        case ISAX_ENVELOPE: {
            IndexAnalyzer<FinalizedISaxIndex<EnvelopeTag>, EnvelopeTag>::analyze_run_index(index_format, num_l_groups,
                                                                                           separate_segment_stats);
            break;
        }
        case ENVELOPE:
        case SAX_ENVELOPE: {
            IndexAnalyzer<FinalizedFlatEnvelopeIndex, EnvelopeTag>::analyze_run_index(index_format, num_l_groups,
                                                                                      separate_segment_stats);
            break;
        }
        case ISAX_ENV_W_ENV:
        case ISAX_ENV_W_SAX_ENV: {
            IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag>::analyze_run_index(index_format, num_l_groups,
                                                                                            separate_segment_stats);
            break;
        }
        default:
            std::cerr << "Statistics are not implemented for index type \"" << SEARCH_METHOD_TYPE_TO_STR.at(method_type)
                      << "\"\n";
            return 2;
    }

    return 0;
}
