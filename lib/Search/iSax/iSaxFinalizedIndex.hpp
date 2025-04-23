#ifndef ISAX_FINALIZED_INDEX_HPP
#define ISAX_FINALIZED_INDEX_HPP

#include <fstream>
#include <queue>

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Paa.hpp"

/**
 * @brief Priority queue entry, intended to be used in iSaxFinalizedIndex
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
struct PQueueISaxEntry {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;

    Real m_min_dist_squared;
    vec<iSaxType> m_isax_words;
    const iSaxFinalizedNode<FTag>* m_node;

    bool operator<(const PQueueISaxEntry& other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

/**
 * @brief Finalized iSAX index
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class iSaxFinalizedIndex : public IFinalizedIndex<FTag> {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

   public:
    iSaxFinalizedIndex() = default;

    /**
     * @brief Construct a new iSaxFinalizedIndex object
     *
     * @param segmentation_strategy The segmentation strategy to use
     * @param first_layer_symbols Symbols of the first layer
     * @param first_layer_nodes First layer nodes
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param alphabet_num_bits The maximum number of bits used for any symbol in any node of the index
     * @param breakpoints Breakpoints used for the iSAX index; assumed to be `2^alphabet_num_bits-1` long;
     *        does not include `-inf` and `inf`
     * @param pos_per_env Number of positions per envelope. 0 if not applicable.
     */
    iSaxFinalizedIndex(sptr<ISegmentationStrategy> segmentation_strategy, vec<vec<vec<SymbolType>>> first_layer_symbols,
                       vec<uptr<iSaxFinalizedNode<FTag>>> first_layer_nodes, SaxNumBitsT first_layer_num_bits,
                       SaxNumBitsT alphabet_num_bits, vec<Real> breakpoints, uint pos_per_env = 0)
        : m_segmentation_strategy(segmentation_strategy),
          m_pos_per_env(pos_per_env),
          m_first_layer_symbols(std::move(first_layer_symbols)),
          m_first_layer_nodes(std::move(first_layer_nodes)),
          m_first_layer_num_bits(first_layer_num_bits),
          m_alphabet_num_bits(alphabet_num_bits),
          m_breakpoints(breakpoints) {  // TODO: try to remove this copy
        assert(m_first_layer_symbols.size() > 0);
    }

    ~iSaxFinalizedIndex() = default;

    /**
     * @brief Get the lower and upper segment limits of the given symbol
     * @param num_bits Number of bits used for the symbol
     * @param symbol The symbol to get the limits for
     * @return A pair of lower and upper limits for the segment
     */
    std::pair<Real, Real> get_segment_limits(SaxNumBitsT num_bits, SymbolType symbol) const {
        uint num_shift = m_alphabet_num_bits - num_bits;
        auto [lower_ind, upper_ind] = get_limit_breakpoint_indexes(symbol, num_shift);
        return {
            lower_ind == -1 ? -INF : m_breakpoints[U(lower_ind)],
            upper_ind == m_breakpoints.size() ? INF : m_breakpoints[U(upper_ind)],
        };
    }

    /**
     * @brief Get the iSAX words of the left and right children after splitting on the given split index
     * @param node The node to get the child iSAX words from
     * @param isax_words The iSAX words of the node
     * @param c The channel index of the split
     * @param s The segment index of the split
     * @return A pair of iSAX words for the left and right children
     */
    std::pair<vec<iSaxType>, vec<iSaxType>> get_children_isax_words(const iSaxFinalizedNode<FTag>* node,
                                                                    vec<iSaxType> isax_words, MtsNumChannelsT c,
                                                                    SaxSegIndT s) const;

    /**
     * @brief Get the symbols of the nodes in the first layer of the index
     * @return The symbols of the nodes in the first layer
     */
    const vec<vec<vec<SymbolType>>>& get_first_layer_symbols() const { return m_first_layer_symbols; }

    /**
     * @brief Get the number of bits used by the nodes in the first layer of the index
     * @return The number of bits used by the nodes in the first layer
     */
    SaxNumBitsT get_first_layer_num_bits() const { return m_first_layer_num_bits; }

    /**
     * @brief Get the first layer node of the index at the given index
     * @param ind The index of the node in teh first layer
     * @return The first layer node at the given index
     */
    const iSaxFinalizedNode<FTag>* get_first_layer_node(size_t ind) const { return m_first_layer_nodes[ind].get(); }

    /**
     * @brief Get the number of positions per envelope
     * @return The number of positions per envelope
     */
    uint get_pos_per_env() const { return m_pos_per_env; }

    /**
     * @brief Get the segmentation strategy
     * @return The segmentation strategy
     */
    const ISegmentationStrategy* get_segmentation_strategy() const { return m_segmentation_strategy.get(); }

   private:
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    uint m_pos_per_env;
    sptr<ISegmentationStrategy> m_segmentation_strategy;
    vec<vec<vec<SymbolType>>> m_first_layer_symbols;
    vec<uptr<iSaxFinalizedNode<FTag>>> m_first_layer_nodes;
    vec<Real> m_breakpoints;

    std::pair<int, int> get_limit_breakpoint_indexes(SymbolType symbol, uint num_shift) const;

    MAKE_SERIALIZABLE((m_first_layer_num_bits, m_alphabet_num_bits, m_pos_per_env, m_segmentation_strategy,
                       m_first_layer_symbols, m_first_layer_nodes, m_breakpoints));
};

/**
 * @brief iSAX index search method
 * @tparam FTag The traits of the entries in the index
 * @tparam S The search type
 * @tparam D The distance type
 * @tparam QS Whether to sort the query or not
 */
template <typename FTag, SearchType S, DistanceType D, bool QS = false>
    requires ValidEntryTraitsTag<FTag>
class iSaxIndexSearch : public IndexSearchMethod<FTag, S, D, QS> {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

   public:
    /**
     * @brief Construct a new iSaxIndexSearch object
     * @param index The iSAX index to use for searching
     */
    iSaxIndexSearch(uptr<iSaxFinalizedIndex<FTag>> index) : m_index(std::move(index)) {}

    SearchResults search(const vec<vec<Real>>& query, const SearchOptions& opts, ResultSet<S>& result_set,
                         const DistanceMeasure<S, D, QS>& distance_measure, std::ifstream& dataset_ifs,
                         const vec<uint>* real_query_inds) const override {
        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(query.size());
        SaxNumBitsT first_layer_num_bits = m_index->get_first_layer_num_bits();
        auto& first_layer_symbols = m_index->get_first_layer_symbols();
        auto segmentation_strategy = m_index->get_segmentation_strategy();
        uint pos_per_env = m_index->get_pos_per_env();

        assert(query.size() == num_channels);

        auto& logger = QueryLogger::get_instance();

        std::priority_queue<PQueueISaxEntry<FTag>> pq;

        auto [query_paa, query_len] = this->get_query_paa_and_len(query, segmentation_strategy, real_query_inds);

        // Go over first layer, calculate MINDIST and iSAX words, push to priority queue
        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (size_t i = 0; i < first_layer_symbols.size(); ++i) {
            Real min_dist_squared = 0;
            vec<iSaxType> isax_words(num_channels);

            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                for (SaxSegIndT s = 0; s < query_paa[c].size(); ++s) {
                    auto [lower, upper] =
                        m_index->get_segment_limits(first_layer_num_bits, first_layer_symbols[i][c][s]);
                    Real segment_len_r = R(segmentation_strategy->get_segment_len(s));
                    min_dist_squared +=
                        distance_measure.min_dist_squared(query_paa[c][s], lower, upper) * segment_len_r;
                }
                isax_words[c] = iSaxType(first_layer_symbols[i][c], first_layer_num_bits);
            }
            pq.push({min_dist_squared, isax_words, m_index->get_first_layer_node(i)});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED, U(pq.size()));

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        size_t leaves_visited = 0, min_dist_seg_updates = 0;
        bool exact_results_found = false;
        while (!pq.empty()) {
            auto [min_dist_squared, isax_words, node] = pq.top();
            pq.pop();

            if (min_dist_squared >= result_set.get_distance_lb()) {
                exact_results_found = true;
                break;
            }

            if (!(node->is_leaf())) {
                auto [s, c] = node->get_split_ind();
                auto [left, right] = node->get_children();

                if (query[c].empty() || query_paa[c].size() <= s) {
                    pq.push({min_dist_squared, isax_words, left});
                    pq.push({min_dist_squared, isax_words, right});
                } else {
                    SaxNumBitsT num_bits = isax_words[c].get_num_bits()[s];
                    Real segment_len_r = R(segmentation_strategy->get_segment_len(s));
                    auto limits = m_index->get_segment_limits(num_bits, isax_words[c].symbol_no_shift(s));
                    Real prev_dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    ++num_bits;

                    auto [left_isax_words, right_isax_words] = m_index->get_children_isax_words(node, isax_words, c, s);

                    // Left child
                    limits = m_index->get_segment_limits(num_bits, left_isax_words[c].symbol_no_shift(s));
                    Real dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len_r * (dist - prev_dist), left_isax_words, left});
                    ++min_dist_seg_updates;

                    // Right child
                    limits = m_index->get_segment_limits(num_bits, right_isax_words[c].symbol_no_shift(s));
                    dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len_r * (dist - prev_dist), right_isax_words, right});
                    ++min_dist_seg_updates;
                }
            } else {
                bool updated = false;

                vec<SubsequenceInfo> subsequence_infos = node->get_subsequence_infos();
                uint entries_checked = 0;
                for (SubsequenceInfo subs_info : subsequence_infos) {
                    if (this->skip_entry(query_len, series_len, subs_info)) continue;

                    size_t data_to_read;
                    if constexpr (std::is_same_v<FTag, EnvelopeTag>) {
                        data_to_read = std::min(subs_info.m_length, query_len + pos_per_env - 1);
                    } else {
                        data_to_read = subs_info.m_length;
                    }

                    vec<vec<Real>> subsequence(num_channels);
                    logger.start_timer(QC::IO_TIME_S);
                    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                        if (query[c].empty()) continue;

                        subsequence[c].resize(data_to_read);
                        dataset_ifs.seekg(subs_info.get_file_pos(series_len, num_channels, c));
                        dataset_ifs.read(reinterpret_cast<char*>(subsequence[c].data()),
                                         static_cast<std::streamsize>(data_to_read * sizeof(Real)));
                    }
                    logger.stop_timer(QC::IO_TIME_S);

                    logger.start_timer(QC::TS_EXAMINATION_TIME_S);
                    updated |=
                        distance_measure.update_result_set(result_set, subs_info, query, subsequence, real_query_inds);
                    logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

                    ++entries_checked;
                }
                logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED, entries_checked);
                logger.increment_count_col(QC::NUM_LEAVES_VISITED);

                if (!opts.m_exact && (++leaves_visited >= opts.m_max_leaves_to_visit || !updated)) break;
            }
            logger.increment_count_col(QC::NUM_NODES_VISITED);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        // TODO: think of some more accurate way of measuring this if needed
        logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED, U(min_dist_seg_updates));

        exact_results_found |= pq.empty();
        return {result_set.get_results(), exact_results_found};
    }

   private:
    uptr<iSaxFinalizedIndex<FTag>> m_index;
};

#endif  // ISAX_FINALIZED_INDEX_HPP
