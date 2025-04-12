#ifndef INDEX_STATS_HPP
#define INDEX_STATS_HPP

#include "Search/Options/SearchMethodType.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

template <typename IndexType, typename FTag>
concept ValidIndexType = std::is_base_of<IFinalizedIndex<FTag>, IndexType>::value;

template <typename IndexType, typename FTag>
    requires ValidIndexType<IndexType, FTag>
class IndexAnalyzer {
   public:
    IndexAnalyzer(uptr<IndexType> index) : m_index(std::move(index)) {};

    void analyze();

   private:
    uptr<IndexType> m_index;

    // iSAX

    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

    void analyze_isax_node(const iSaxFinalizedIndex<FTag> *index, const iSaxFinalizedNode<FTag> *node,
                           const vec<iSaxType> &isax_words, IndexStats &stats, size_t height) {
        if (node->is_leaf()) {
            size_t num_entries = node->get_subsequence_infos().size();
            stats.update_leaf_stats(num_entries, height);
            for (MtsNumChannelsT c = 0; c < isax_words.size(); ++c) {
                auto channel_num_bits = isax_words[c].get_num_bits();
                for (SaxSegIndT s = 0; s < isax_words[c].size(); ++s) {
                    auto [lower, upper] =
                        index->get_segment_limits(channel_num_bits[s], isax_words[c].symbol_no_shift(s));
                    stats.update_seg_stats(lower, upper, num_entries);
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

    void analyze_isax() {
        const iSaxFinalizedIndex<FTag> *index = dynamic_cast<iSaxFinalizedIndex<FTag> *>(m_index.get());
        if (!index) throw std::runtime_error("Could not cast index to iSaxFinalizedIndex");

        auto &RS = RunSettings::get_instance();
        MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;

        IndexStats stats;

        const auto &first_layer_symbols = index->get_first_layer_symbols();
        for (size_t i = 0; i < first_layer_symbols.size(); ++i) {
            vec<iSaxType> isax_words(num_channels);
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                isax_words[c] = iSaxType(first_layer_symbols[i][c], index->get_first_layer_num_bits());
            }
            analyze_isax_node(index, index->get_first_layer_node(i), isax_words, stats, 1);
        }
        stats.calculate();
        IndexStatsLogger::write_entry(stats);
    }
};

int calculate_index_stats(SearchMethodType method_type, ArchiveType index_format);

#endif  // INDEX_STATS_HPP
