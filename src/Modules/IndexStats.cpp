#include "Modules/IndexStats.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

// Utility

// iSAX

template <>
void IndexAnalyzer<iSaxFinalizedIndex<PaaTag>, PaaTag>::analyze() {
    analyze_isax();
}

// iSAX + envelope

template <>
void IndexAnalyzer<iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag>::analyze() {
    analyze_isax();
}

// Envelope

template <>
void IndexAnalyzer<FlatEnvelopeIndex, EnvelopeTag>::analyze() {
    const FlatEnvelopeIndex *index = dynamic_cast<FlatEnvelopeIndex *>(m_index.get());
    if (!index) throw std::runtime_error("Could not cast index to FlatEnvelopeIndex");

    auto &RS = RunSettings::get_instance();
    MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;

    IndexStats stats;
    for (const IndexEntry<Envelope> &entry : index->get_entries()) {
        stats.update_leaf_stats(1, 1);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto &summary = entry.m_mts_summary[c];
            for (SaxSegIndT s = 0; s < summary.m_lower.size(); ++s) {
                Real lower = summary.m_lower[s], upper = summary.m_upper[s];
                stats.update_seg_stats(lower, upper);
            }
        }
    }
    stats.calculate();
    IndexStatsLogger::write_entry(stats);
}

// Main

int calculate_index_stats(SearchMethodType method_type, ArchiveType index_format) {
    auto &RS = RunSettings::get_instance();

#define ANALYZE_INDEX(INDEX_TYPE, F_TAG)                              \
    try {                                                             \
        auto index_file = RS.get_index_path();                        \
        index_file = add_archive_extension(index_file, index_format); \
        auto index = std::make_unique<INDEX_TYPE>();                  \
        index->load(index_file, index_format);                        \
        using IndexType = INDEX_TYPE;                                 \
        IndexAnalyzer<IndexType, F_TAG> analyzer(std::move(index));   \
        analyzer.analyze();                                           \
    } catch (const std::exception &e) {                               \
        std::cerr << "Error analyzing index: " << e.what() << '\n';   \
        return 1;                                                     \
    }

    switch (method_type) {
        case ISAX: {
            ANALYZE_INDEX(iSaxFinalizedIndex<PaaTag>, PaaTag);
            break;
        }
        case ISAX_ENVELOPE: {
            ANALYZE_INDEX(iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag);
            break;
        }
        case ENVELOPE: {
            ANALYZE_INDEX(FlatEnvelopeIndex, EnvelopeTag);
            break;
        }
        default:
            std::cerr << "Statistics are not implemented for index type \"" << SEARCH_METHOD_TYPE_TO_STR.at(method_type)
                      << "\"\n";
            return 2;
    }

    return 0;
}
