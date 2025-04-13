#include "Modules/IndexStats.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/ChainIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/Paa.hpp"

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

// Envelope, SAX envelope

template <>
void IndexAnalyzer<FlatEnvelopeIndex, EnvelopeTag>::analyze() {
    const FlatEnvelopeIndex *index = dynamic_cast<FlatEnvelopeIndex *>(m_index.get());
    if (!index) throw std::runtime_error("Could not cast index to FlatEnvelopeIndex");

    auto &RS = RunSettings::get_instance();
    MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;
    SaxSegIndT num_seg_per_channel = index->get_num_seg_per_channel();

    IndexStats stats(num_channels, num_seg_per_channel);
    for (const IndexEntry<Envelope> &entry : index->get_entries()) {
        stats.update_leaf_stats(1, 1);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto &summary = entry.m_mts_summary[c];
            for (SaxSegIndT s = 0; s < summary.m_lower.size(); ++s) {
                Real lower = summary.m_lower[s], upper = summary.m_upper[s];
                stats.update_seg_stats(lower, upper, c, s);
            }
        }
    }
    stats.calculate();
    IndexStatsLogger::write_entry(stats, m_sub_index_id);
}

// Main

int calculate_index_stats(SearchMethodType method_type, ArchiveType index_format) {
    auto &RS = RunSettings::get_instance();

#define LOAD_INDEX                                                    \
    try {                                                             \
        auto index_file = RS.get_index_path();                        \
        index_file = add_archive_extension(index_file, index_format); \
        index->load(index_file, index_format);                        \
    } catch (const std::exception &e) {                               \
        std::cerr << "Error loading index: " << e.what() << '\n';     \
        return 1;                                                     \
    }

#define ANALYZE_SIMPLE_INDEX(INDEX_TYPE, F_TAG)  \
    auto index = std::make_unique<INDEX_TYPE>(); \
    LOAD_INDEX;                                  \
    IndexAnalyzer<INDEX_TYPE, F_TAG>(std::move(index)).analyze();

    switch (method_type) {
        case ISAX: {
            ANALYZE_SIMPLE_INDEX(iSaxFinalizedIndex<PaaTag>, PaaTag);
            break;
        }
        case ISAX_ENVELOPE: {
            ANALYZE_SIMPLE_INDEX(iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag);
            break;
        }
        case ENVELOPE:
        case SAX_ENVELOPE: {
            ANALYZE_SIMPLE_INDEX(FlatEnvelopeIndex, EnvelopeTag);
            break;
        }
        case ISAX_ENV_W_ENV:
        case ISAX_ENV_W_SAX_ENV: {
            uptr<ChainFinalizedIndex<EnvelopeTag>> index;
            {
                vec<uptr<IFinalizedIndex<EnvelopeTag>>> approx_indexes(1);
                approx_indexes[0] = std::make_unique<iSaxFinalizedIndex<EnvelopeTag>>();
                auto exact_index = uptr<IFinalizedIndex<EnvelopeTag>>(new FlatEnvelopeIndex());
                index = std::make_unique<ChainFinalizedIndex<EnvelopeTag>>(std::move(approx_indexes),
                                                                           std::move(exact_index));
            }
            LOAD_INDEX;

            auto approx_index = uptr<iSaxFinalizedIndex<EnvelopeTag>>(
                static_cast<iSaxFinalizedIndex<EnvelopeTag> *>(index->release_approx_index(0)));
            IndexAnalyzer<iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag>(std::move(approx_index), 0u).analyze();

            auto exact_index = uptr<FlatEnvelopeIndex>(static_cast<FlatEnvelopeIndex *>(index->release_exact_index()));
            IndexAnalyzer<FlatEnvelopeIndex, EnvelopeTag>(std::move(exact_index), 1u).analyze();
            break;
        }
        default:
            std::cerr << "Statistics are not implemented for index type \"" << SEARCH_METHOD_TYPE_TO_STR.at(method_type)
                      << "\"\n";
            return 2;
    }

    return 0;
}
