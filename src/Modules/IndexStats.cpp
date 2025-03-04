#include "Modules/IndexStats.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"

// Utility

// iSAX

template <>
void IndexAnalyzer<iSaxFinalizedIndex<PaaTag>, PaaTag>::analyze() {}

// iSAX + envelope

template <>
void IndexAnalyzer<iSaxFinalizedIndex<EnvelopeTag>, EnvelopeTag>::analyze() {}

// Envelope

template <>
void IndexAnalyzer<FlatEnvelopeIndex, EnvelopeTag>::analyze() {}

// Main

#define ANALYZE_INDEX(INDEX_TYPE, F_TAG)                        \
    auto index_stream = RS.get_index_ifs();                     \
    auto index = std::make_unique<INDEX_TYPE>();                \
    index->load(index_stream, index_format);                    \
    using IndexType = INDEX_TYPE;                               \
    IndexAnalyzer<IndexType, F_TAG> analyzer(std::move(index)); \
    analyzer.analyze();

int calculate_index_stats(SearchMethodType method_type, ArchiveType index_format) {
    auto &RS = RunSettings::get_instance();

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
            return 1;
    }

    return 0;
}
