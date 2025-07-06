#ifndef UTIL_STATS_INDEXANALYZER_HPP
#define UTIL_STATS_INDEXANALYZER_HPP

#include <type_traits>

#include "Index/FinalizedIndex.hpp"
#include "Index/LengthGroupingIndex/FinalizedLengthGroupingIndex.hpp"
#include "Index/Traits/SaxTraits.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Util/Logging/IndexStatsLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Stats/IndexStats.hpp"
#include "Util/Types/Pointers.hpp"

template <typename IndexType, typename FTag>
concept ValidIndexType = std::is_base_of<IFinalizedIndex<FTag>, IndexType>::value;

/**
 * @brief Class for analyzing indexes
 * @tparam IndexType The type of index to analyze, must extend IFinalizedIndex
 * @tparam FTag The finalized tag of the index, must be a valid entry traits
 * @tparam EnvT The type of envelope in the index if applicable, defaults to void
 */
template <typename IndexType, typename FTag, typename EnvT = void>
    requires ValidIndexType<IndexType, FTag>
class IndexAnalyzer {
   public:
    IndexAnalyzer(uptr<IndexType> index, uint sub_index_id = 0)
        : m_index(std::move(index)), m_sub_index_id(sub_index_id) {};

    /**
     * @brief Analyze the index, write the results into the log file
     * @param length_group_id The ID of the length group within the index (0 for non-length-grouped indexes)
     * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
     */
    void analyze(uint length_group_id = 0, bool separate_segment_stats = false);

    /**
     * @brief Analyze the index of the current run, write the results into the log file
     * @param index_format The format of the index
     * @param num_l_groups The number of length groups to use (0 for non-length-grouped indexes)
     * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
     */
    static void analyze_run_index(ArchiveType index_format, uint num_l_groups = 0,
                                  bool separate_segment_stats = false) {
        if (num_l_groups > 0) {
            vec<uptr<IFinalizedIndex<FTag>>> group_indexes(num_l_groups);
            for (uint l_ind = 0; l_ind < num_l_groups; l_ind++) group_indexes[l_ind] = create_index();
            auto index =
                std::make_unique<FinalizedLengthGroupingIndex<FTag>>(std::move(group_indexes), LengthProperties{});
            IndexAnalyzer<FinalizedLengthGroupingIndex<FTag>, FTag>::load_index(index, index_format);

            for (uint l_ind = 0; l_ind < num_l_groups; l_ind++) {
                auto sub_index = uptr<IndexType>(static_cast<IndexType *>(index->release_index(l_ind)));
                IndexAnalyzer<IndexType, FTag, EnvT>(std::move(sub_index)).analyze(l_ind, separate_segment_stats);
            }
        } else {
            auto index = create_index();
            load_index(index, index_format);
            IndexAnalyzer<IndexType, FTag, EnvT>(std::move(index)).analyze(0, separate_segment_stats);
        }
    }

    /**
     * @brief Load the index
     * @param index The index to load
     * @param index_format The format of the index
     */
    static void load_index(uptr<IndexType> &index, ArchiveType index_format) {
        auto &RS = RunSettings::get_instance();
        try {
            auto index_file = RS.get_index_path();
            index->load(index_file, index_format);
        } catch (const std::exception &e) {
            throw std::runtime_error("Error loading index" + std::string(e.what()) + '\n');
        }
    }

   private:
    uptr<IndexType> m_index;
    uint m_sub_index_id;

    // Index creation

    /**
     * @brief Create an index to analyze (with default parameters)
     * @return A unique pointer to the new index
     */
    static uptr<IndexType> create_index() { return std::make_unique<IndexType>(); }

    // iSAX

    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

    /**
     * @brief Analyze a node of the iSAX index
     * @param index The iSAX index
     * @param node The node to analyze
     * @param isax_words The iSAX words of the node
     * @param stats The statistics to update
     * @param height The height of the node in the index
     */
    void analyze_isax_node(const FinalizedISaxIndex<FTag> *index, const FinalizedISaxNode<FTag> *node,
                           const vec<iSaxType> &isax_words, IndexStats &stats, size_t height);

    /**
     * @brief Analyze iSAX index
     * @param length_group_id The ID of the length group within the index (0 for non-length-grouped indexes)
     * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
     * */
    void analyze_isax(uint length_group_id = 0, bool separate_segment_stats = false);

    // Flat envelope

    /**
     * @brief Analyze flat envelope index
     * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
     * */
    void analyze_flat_envelope(uint length_group_id = 0, bool separate_segment_stats = false);

    // Two-stage iSAX with envelope / SAX envelope

    /**
     * @brief Analyze two-stage iSAX envelope index
     * @param length_group_id The ID of the length group within the index (0 for non-length-grouped indexes)
     * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
     */
    void analyze_two_stage_isax_envelope(uint length_group_id = 0, bool separate_segment_stats = false);
};

#endif  // UTIL_STATS_INDEXANALYZER_HPP
