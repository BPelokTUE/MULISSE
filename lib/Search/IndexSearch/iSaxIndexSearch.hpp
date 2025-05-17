#ifndef SEARCH_INDEXSEARCH_ISAXINDEXSEARCH_HPP
#define SEARCH_INDEXSEARCH_ISAXINDEXSEARCH_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Search/IndexSearch/IndexSearchMethod.hpp"
#include "Search/Results/ResultSet.hpp"
#include "Util/Logging/QueryLogger.hpp"

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
    iSaxIndexSearch(uptr<FinalizedISaxIndex<FTag>> index) : m_index(std::move(index)) {}

    SearchResults search(const vec<vec<Real>>& query, const SearchOptions& opts, ResultSet<S>& result_set,
                         const DistanceMeasure<S, D, QS>& distance_measure, std::ifstream& dataset_ifs,
                         const vec<uint>* real_query_inds) const override {
        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(query.size());
        SaxNumBitsT first_layer_num_bits = m_index->get_first_layer_num_bits();
        auto& first_layer_symbols = m_index->get_first_layer_symbols();
        auto ch_segmentation_strategy = m_index->get_ch_segmentation_strategy();

        assert(query.size() == num_channels);

        auto& logger = QueryLogger::get_instance();

        std::priority_queue<PQueueISaxEntry<FTag>> pq;

        auto [query_paa, query_len] = this->get_query_paa_and_len(query, ch_segmentation_strategy, real_query_inds);

        // Go over first layer, calculate MINDIST and iSAX words, push to priority queue
        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (size_t i = 0; i < first_layer_symbols.size(); ++i) {
            Real min_dist_squared = 0;
            vec<iSaxType> isax_words(num_channels);

            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                auto segmentation_strategy = ch_segmentation_strategy->get_const_segmentation_strategy(c);
                for (SaxSegIndT s = 0; s < query_paa[c].size(); ++s) {
                    auto [lower, upper] =
                        m_index->get_interval_limits(first_layer_num_bits, first_layer_symbols[i][c][s]);
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
                    Real segment_len_r = R(ch_segmentation_strategy->get_segmentation_strategy(c)->get_segment_len(s));
                    auto limits = m_index->get_interval_limits(num_bits, isax_words[c].symbol_no_shift(s));
                    Real prev_dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    ++num_bits;

                    auto [left_isax_words, right_isax_words] = m_index->get_children_isax_words(node, isax_words, c, s);

                    // Left child
                    limits = m_index->get_interval_limits(num_bits, left_isax_words[c].symbol_no_shift(s));
                    Real dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len_r * (dist - prev_dist), left_isax_words, left});
                    ++min_dist_seg_updates;

                    // Right child
                    limits = m_index->get_interval_limits(num_bits, right_isax_words[c].symbol_no_shift(s));
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
                        uint num_start_pos = subs_info.m_length;
                        data_to_read = std::min(query_len + num_start_pos - 1, series_len - subs_info.m_start_pos);
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
    uptr<FinalizedISaxIndex<FTag>> m_index;
};

#endif  // SEARCH_INDEXSEARCH_ISAXINDEXSEARCH_HPP
