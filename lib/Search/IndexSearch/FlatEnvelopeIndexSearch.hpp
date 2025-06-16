#ifndef SEARCH_INDEXSEARCH_FLATENVELOPEINDEXSEARCH_HPP
#define SEARCH_INDEXSEARCH_FLATENVELOPEINDEXSEARCH_HPP

#include "Search/IndexSearch/EnvelopeIndexSearch.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

/** @brief Priority queue entry for searching with FlatEnvelopeIndexSearch */
struct PQueueEnvelopeEntry {
    Real m_min_dist_squared;
    SubsequenceInfo m_subs_info;

    bool operator<(const PQueueEnvelopeEntry &other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

/**
 * @brief Flat envelope index search method
 * @tparam EnvT Envelope type to use
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 */
template <typename EnvT, SearchType S, DistanceType D, bool QS = false>
class FlatEnvelopeIndexSearch : public EnvelopeIndexSearch<S, D, QS> {
   public:
    FlatEnvelopeIndexSearch(uptr<FinalizedFlatEnvelopeIndex<EnvT>> index, bool use_priority_queue = true)
        : m_index(std::move(index)), m_use_priority_queue(use_priority_queue) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto [query_paa, query_len] =
            this->get_query_paa_and_len(query, m_index->get_ch_segmentation_strategy(), real_query_inds);

        if (m_use_priority_queue) {
            return search_with_priority_queue(query, query_paa, query_len, result_set, distance_measure, dataset_ifs,
                                              real_query_inds);
        } else {
            return search_sequentially(query, query_paa, query_len, result_set, distance_measure, dataset_ifs,
                                       real_query_inds);
        }
    };

   private:
    inline SearchResults search_with_priority_queue(const vec<vec<Real>> &query, const vec<vec<Real>> &query_paa,
                                                    uint query_len, ResultSet<S> &result_set,
                                                    const DistanceMeasure<S, D, QS> &distance_measure,
                                                    std::ifstream &dataset_ifs,
                                                    const vec<uint> *real_query_inds) const {
        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        auto &logger = QueryLogger::get_instance();

        std::priority_queue<PQueueEnvelopeEntry> pq;
        auto ch_segmentation_strategy = m_index->get_ch_segmentation_strategy();

        logger.start_timer(QC::FIRST_LAYER_TIME_S);

        Real min_dist_total = 0;
        uint num_entries = m_index->size();
        for (uint i = 0; i < num_entries; ++i) {
            const auto &entry = m_index->get_entry(i);
            if (this->skip_entry(query_len, series_len, entry.m_subs_info)) continue;

            Real min_dist_squared =
                this->get_min_dist_squared(entry.m_mts_summary, query_paa, distance_measure, ch_segmentation_strategy);
            pq.push({min_dist_squared, entry.m_subs_info});
            min_dist_total += min_dist_squared;
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.set_number_col(QC::MIN_DIST_TOTAL, min_dist_total);

        logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED, U(pq.size()));

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        while (!pq.empty()) {
            auto [min_dist_squared, subs_info] = pq.top();
            pq.pop();

            if (min_dist_squared >= result_set.get_distance_lb()) break;
            this->update_result_set(subs_info, m_index->get_pos_per_env(), query, query_len, result_set,
                                    distance_measure, dataset_ifs, real_query_inds);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        return {result_set.get_results(), true};
    }

    inline SearchResults search_sequentially(const vec<vec<Real>> &query, const vec<vec<Real>> &query_paa,
                                             uint query_len, ResultSet<S> &result_set,
                                             const DistanceMeasure<S, D, QS> &distance_measure,
                                             std::ifstream &dataset_ifs, const vec<uint> *real_query_inds) const {
        auto &logger = QueryLogger::get_instance();
        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        auto ch_segmentation_strategy = m_index->get_ch_segmentation_strategy();

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        uint num_entries = m_index->size();
        for (uint i = 0; i < num_entries; ++i) {
            const auto &entry = m_index->get_entry(i);
            if (this->skip_entry(query_len, series_len, entry.m_subs_info)) continue;

            Real min_dist_squared =
                this->get_min_dist_squared(entry.m_mts_summary, query_paa, distance_measure, ch_segmentation_strategy);
            logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED);
            if (min_dist_squared >= result_set.get_distance_lb()) continue;

            this->update_result_set(entry.m_subs_info, m_index->get_pos_per_env(), query, query_len, result_set,
                                    distance_measure, dataset_ifs, real_query_inds);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        return {result_set.get_results(), true};
    }

    uptr<FinalizedFlatEnvelopeIndex<EnvT>> m_index;
    bool m_use_priority_queue;
};

#endif  // SEARCH_INDEXSEARCH_FLATENVELOPEINDEXSEARCH_HPP
