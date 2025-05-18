#ifndef SEARCH_INDEXSEARCH_TREEENVELOPEINDEXSEARCH_HPP
#define SEARCH_INDEXSEARCH_TREEENVELOPEINDEXSEARCH_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Index/EnvelopeIndex/Tree/FinalizedTreeEnvelopeIndex.hpp"
#include "Search/IndexSearch/EnvelopeIndexSearch.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Priority queue entry for searching with TreeEnvelopeIndexSearch */
struct PQueueEnvelopeNodeEntry {
    Real m_min_dist_squared;
    const EnvelopeNode *m_node;

    bool operator<(const PQueueEnvelopeNodeEntry &other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

template <SearchType S, DistanceType D, bool QS = false>
class TreeEnvelopeIndexSearch : public EnvelopeIndexSearch<S, D, QS> {
   public:
    TreeEnvelopeIndexSearch(uptr<FinalizedTreeEnvelopeIndex> index) : m_index(std::move(index)) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto &logger = QueryLogger::get_instance();

        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        auto ch_segmentation_strategy = m_index->get_ch_segmentation_strategy();
        auto [query_paa, query_len] = this->get_query_paa_and_len(query, ch_segmentation_strategy, real_query_inds);

        std::priority_queue<PQueueEnvelopeNodeEntry> pq;

        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (auto &node : m_index->get_first_layer_nodes()) {
            Real min_dist_squared = this->get_min_dist_squared(node->get_envelopes(), query_paa, distance_measure,
                                                               ch_segmentation_strategy);
            pq.push({min_dist_squared, node});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED, U(pq.size()));

        while (!pq.empty()) {
            auto entry = pq.top();
            pq.pop();

            Real lb = result_set.get_distance_lb();
            if (entry.m_min_dist_squared >= lb) break;

            if (!entry.m_node->is_leaf()) {
                for (auto &child : entry.m_node->get_children()) {
                    Real min_dist_squared = this->get_min_dist_squared(child->get_envelopes(), query_paa,
                                                                       distance_measure, ch_segmentation_strategy);
                    logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED);
                    if (min_dist_squared < lb) pq.push({min_dist_squared, child});
                }
            } else {
                for (auto subs_info : *(entry.m_node->get_subsequence_infos())) {
                    if (this->skip_entry(query_len, series_len, subs_info)) continue;
                    this->update_result_set(subs_info, m_index->get_pos_per_env(), query, query_len, result_set,
                                            distance_measure, dataset_ifs, real_query_inds);
                }
            }
        }
        return {result_set.get_results(), true};
    }

   private:
    uptr<FinalizedTreeEnvelopeIndex> m_index;
};

#endif  // SEARCH_INDEXSEARCH_TREEENVELOPEINDEXSEARCH_HPP
