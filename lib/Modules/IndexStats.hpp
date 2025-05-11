#ifndef MODULES_INDEXSTATS_HPP
#define MODULES_INDEXSTATS_HPP

#include <type_traits>

#include "Enums/ArchiveType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Index/FinalizedIndex.hpp"
#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Util/Logging/IndexStatsLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Pointers.hpp"

template <typename IndexType, typename FTag>
concept ValidIndexType = std::is_base_of<IFinalizedIndex<FTag>, IndexType>::value;

template <typename IndexType, typename FTag>
    requires ValidIndexType<IndexType, FTag>
class IndexAnalyzer {
   public:
    IndexAnalyzer(uptr<IndexType> index, uint sub_index_id = 0)
        : m_index(std::move(index)), m_sub_index_id(sub_index_id) {};

    /**
     * @brief Analyze the index, write the results into the log file
     * @param length_group_id The ID of the length group within the index (0 for non-length-grouped indexes)
     */
    void analyze(uint length_group_id = 0);

    /**
     * @brief Analyze the index of the current run, write the results into the log file
     * @param index_format The format of the index
     * @param num_l_groups The number of length groups to use (0 for non-length-grouped indexes)
     */
    static void analyze_run_index(ArchiveType index_format, uint num_l_groups = 0) {
        if (num_l_groups > 0) {
            vec<uptr<IFinalizedIndex<FTag>>> group_indexes(num_l_groups);
            for (uint l_ind = 0; l_ind < num_l_groups; l_ind++) group_indexes[l_ind] = create_index();
            auto index = std::make_unique<FinalizedLengthGroupingIndex<FTag>>(std::move(group_indexes), 1, 1);
            IndexAnalyzer<FinalizedLengthGroupingIndex<FTag>, FTag>::load_index(index, index_format);

            for (uint l_ind = 0; l_ind < num_l_groups; l_ind++) {
                auto sub_index = uptr<IndexType>(static_cast<IndexType *>(index->release_index(l_ind)));
                IndexAnalyzer<IndexType, FTag>(std::move(sub_index)).analyze(l_ind);
            }
        } else {
            auto index = create_index();
            load_index(index, index_format);
            IndexAnalyzer<IndexType, FTag>(std::move(index)).analyze();
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
                           const vec<iSaxType> &isax_words, IndexStats &stats, size_t height) {
        if (node->is_leaf()) {
            size_t num_entries = node->get_subsequence_infos().size();
            stats.update_leaf_stats(num_entries, height);
            for (MtsNumChannelsT c = 0; c < isax_words.size(); ++c) {
                auto channel_num_bits = isax_words[c].get_num_bits();
                for (SaxSegIndT s = 0; s < isax_words[c].size(); ++s) {
                    auto [lower, upper] =
                        index->get_segment_limits(channel_num_bits[s], isax_words[c].symbol_no_shift(s));
                    stats.update_seg_stats(lower, upper, c, s, num_entries);
                }
            }
        } else {
            auto [s, c] = node->get_split_ind();
            auto [left_isax_words, right_isax_words] = index->get_children_isax_words(node, isax_words, c, s);
            auto [left, right] = node->get_children();
            analyze_isax_node(index, left, left_isax_words, stats, height + 1);
            analyze_isax_node(index, right, right_isax_words, stats, height + 1);
        }
    }

    /**
     * @brief Analyze the iSAX index
     * @param length_group_id The ID of the length group within the index (0 for non-length-grouped indexes)
     * */
    void analyze_isax(uint length_group_id = 0) {
        const FinalizedISaxIndex<FTag> *index = dynamic_cast<FinalizedISaxIndex<FTag> *>(m_index.get());
        if (!index) throw std::runtime_error("Could not cast index to FinalizedISaxIndex");

        const auto &first_layer_symbols = index->get_first_layer_symbols();
        if (first_layer_symbols.empty()) {
            throw std::runtime_error("First layer symbols are empty");
        }
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(first_layer_symbols[0].size());
        if (num_channels == 0) {
            throw std::runtime_error("Number of channels is 0");
        }
        SaxSegIndT num_segments = static_cast<SaxSegIndT>(first_layer_symbols[0][0].size());
        if (num_segments == 0) {
            throw std::runtime_error("Number of segments is 0");
        }
        IndexStats stats(num_channels, num_segments);

        for (size_t i = 0; i < first_layer_symbols.size(); ++i) {
            vec<iSaxType> isax_words(num_channels);
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                isax_words[c] = iSaxType(first_layer_symbols[i][c], index->get_first_layer_num_bits());
            }
            analyze_isax_node(index, index->get_first_layer_node(i), isax_words, stats, 1);
        }
        stats.calculate();
        IndexStatsLogger::write_entry(stats, length_group_id, m_sub_index_id);
    }
};

/**
 * @brief Calculate index statistics
 * @param method_type The type of index to use
 * @param num_l_groups The number of length groups to use
 * @param index_format The format of the index
 */
int calculate_index_stats(SearchMethodType method_type, uint num_l_groups, ArchiveType index_format);

#endif  // MODULES_INDEXSTATS_HPP
