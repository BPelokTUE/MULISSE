#include "Modules/CalcIndexStats.hpp"

#include "Index/ChainIndex/FinalizedChainIndex.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Util/Stats/IndexAnalyzer.hpp"

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
