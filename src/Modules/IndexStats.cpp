#include "Modules/IndexStats.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logging/IndexStatsLogger.hpp"
#include "Util/RunSettings.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/Envelope/FlatEnvelopeIndex.hpp"
#include "Search/ChainIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/Paa.hpp"

// Utility

// iSAX

template <>
void IndexAnalyzer<iSaxFinalizedIndex<PaaTag>, PaaTag>::analyze(uint length_group_id) {
    analyze_isax(length_group_id);
}

// iSAX + envelope

template <>
void IndexAnalyzer<iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag>::analyze(uint length_group_id) {
    analyze_isax(length_group_id);
}

// Envelope / SAX envelope

template <>
void IndexAnalyzer<FinalizedFlatEnvelopeIndex, EnvelopeTag>::analyze(uint length_group_id) {
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
        throw std::runtime_error("Number of segments is 0");
    }
    IndexStats stats(num_channels, num_segments);

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
    IndexStatsLogger::write_entry(stats, length_group_id, m_sub_index_id);
}

// iSAX + envelope / SAX envelope

template <>
uptr<ChainFinalizedIndex<EnvelopeTag>> IndexAnalyzer<ChainFinalizedIndex<EnvelopeTag>, EnvelopeTag>::create_index() {
    vec<uptr<IFinalizedIndex<EnvelopeTag>>> approx_indexes(1);
    approx_indexes[0] = std::make_unique<iSaxFinalizedIndex<EnvelopeTag>>();
    auto exact_index = uptr<IFinalizedIndex<EnvelopeTag>>(new FinalizedFlatEnvelopeIndex());
    return std::make_unique<ChainFinalizedIndex<EnvelopeTag>>(std::move(approx_indexes), std::move(exact_index));
}

template <>
void IndexAnalyzer<ChainFinalizedIndex<EnvelopeTag>, EnvelopeTag>::analyze(uint length_group_id) {
    auto approx_index = uptr<iSaxFinalizedIndex<EnvelopeTag>>(
        static_cast<iSaxFinalizedIndex<EnvelopeTag> *>(m_index->release_approx_index(0)));
    IndexAnalyzer<iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag>(std::move(approx_index), 0u).analyze(length_group_id);

    auto exact_index =
        uptr<FinalizedFlatEnvelopeIndex>(static_cast<FinalizedFlatEnvelopeIndex *>(m_index->release_exact_index()));
    IndexAnalyzer<FinalizedFlatEnvelopeIndex, EnvelopeTag>(std::move(exact_index), 1u).analyze(length_group_id);
}

// Main

int calculate_index_stats(SearchMethodType method_type, uint num_l_groups, ArchiveType index_format) {
    switch (method_type) {
        case ISAX: {
            IndexAnalyzer<iSaxFinalizedIndex<PaaTag>, PaaTag>::analyze_run_index(index_format, num_l_groups);
            break;
        }
        case ISAX_ENVELOPE: {
            IndexAnalyzer<iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag>::analyze_run_index(index_format, num_l_groups);
            break;
        }
        case ENVELOPE:
        case SAX_ENVELOPE: {
            IndexAnalyzer<FinalizedFlatEnvelopeIndex, EnvelopeTag>::analyze_run_index(index_format, num_l_groups);
            break;
        }
        case ISAX_ENV_W_ENV:
        case ISAX_ENV_W_SAX_ENV: {
            IndexAnalyzer<ChainFinalizedIndex<EnvelopeTag>, EnvelopeTag>::analyze_run_index(index_format, num_l_groups);
            break;
        }
        default:
            std::cerr << "Statistics are not implemented for index type \"" << SEARCH_METHOD_TYPE_TO_STR.at(method_type)
                      << "\"\n";
            return 2;
    }

    return 0;
}
