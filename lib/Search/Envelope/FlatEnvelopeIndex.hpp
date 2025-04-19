#ifndef FLAT_ENVELOPE_INDEX_HPP
#define FLAT_ENVELOPE_INDEX_HPP

#include <queue>

#include "Util/typedefs.hpp"
#include "Search/Index.hpp"
#include "Search/SearchMethod.hpp"
#include "Search/TopDownInserter.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/Options/IndexOptions.hpp"

class FinalizedFlatEnvelopeIndex : public FinalizedEnvelopeIndex {
   public:
    FinalizedFlatEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedFlatEnvelopeIndex instance
     * @param segment_len The length of Paa the segments
     * @param pos_per_env The number of positions per envelope
     * @param entries The envelope entries in the index
     */
    FinalizedFlatEnvelopeIndex(const uint segment_len, const uint pos_per_env, vec<IndexEntry<Envelope>> &&entries)
        : FinalizedEnvelopeIndex(segment_len, pos_per_env), m_entries(std::move(entries)) {}

    const vec<IndexEntry<Envelope>> &get_entries() const { return m_entries; }

    inline SaxSegIndT get_num_seg_per_channel() const {
        return static_cast<SaxSegIndT>(m_entries[0].m_mts_summary[0].m_lower.size());
    }

   private:
    vec<IndexEntry<Envelope>> m_entries;

    MAKE_SERIALIZABLE((m_segment_len, m_pos_per_env, m_entries));
};

/** @brief Flat envelope index */
class FlatEnvelopeIndex : public EnvelopeIndex, public std::enable_shared_from_this<FlatEnvelopeIndex> {
   public:
    /**
     * @brief Construct a new FlatEnvelopeIndex instance
     * @param segment_len Length of the segments
     * @param pos_per_env Number of positions per envelope
     * @param sax_num_bits Number of bits used for SAX discretization. Defaults to 0, indicating no discretization.
     * If greater than 0, the breakpoints are assumed to have the same cardinality.
     */
    FlatEnvelopeIndex(uint segment_len, uint pos_per_env, SaxNumBitsT sax_num_bits = 0)
        : EnvelopeIndex(segment_len, pos_per_env), m_sax_num_bits(sax_num_bits) {}

    FlatEnvelopeIndex() = default;

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override {
        uptr<IEntryInserter<FlatEnvelopeIndex>> inserter;
        switch (inserter_type) {
            case ISAX_PARALLEL:  // Temporary solution to support two-stage indexes
            case TOP_DOWN:
                inserter = std::make_unique<TopDownInserter<FlatEnvelopeIndex>>(this->shared_from_this());
                break;
            default:
                throw std::invalid_argument("Invalid inserter type");
        }
        inserter->insert_entries(entries);
    };

    void insert(IndexEntry<Envelope> &entry) override {
        if (m_sax_num_bits > 0) {
            auto &breakpoints = RunSettings::get_instance().get_breakpoints();
            SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(entry.m_mts_summary[0].m_lower.size());

            for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
                SaxWord sax_lower(entry.m_mts_summary[c].m_lower, m_sax_num_bits, breakpoints);
                SaxWord sax_upper(entry.m_mts_summary[c].m_upper, m_sax_num_bits, breakpoints);

                for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) {
                    entry.m_mts_summary[c].m_lower[s] = sax_lower[s] > 0 ? breakpoints[sax_lower[s] - 1] : -INF;
                    entry.m_mts_summary[c].m_upper[s] =
                        sax_upper[s] < breakpoints.size() ? breakpoints[sax_upper[s]] : INF;
                }
            }
        }
        EnvelopeIndex::insert(entry);
    }

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override {
        return std::make_unique<FinalizedFlatEnvelopeIndex>(m_segment_len, m_pos_per_env, std::move(m_entries));
    }

   protected:
    SaxNumBitsT m_sax_num_bits;
};

/** @brief Priority queue entry for searching with FlatEnvelopeIndexSearch */
struct PQueueEnvelopeEntry {
    Real m_min_dist_squared;
    SubsequenceInfo m_subs_info;

    bool operator<(const PQueueEnvelopeEntry &other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

/**
 * @brief Flat envelope index search method
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 */
template <SearchType S, DistanceType D, bool QS = false>
class FlatEnvelopeIndexSearch : public EnvelopeIndexSearch<S, D, QS> {
   public:
    FlatEnvelopeIndexSearch(uptr<FinalizedFlatEnvelopeIndex> index, bool use_priority_queue = true)
        : m_index(std::move(index)), m_use_priority_queue(use_priority_queue) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto [query_paa, query_len] = this->get_query_paa_and_len(query, m_index->get_segment_len(), real_query_inds);

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
        Real segment_len_r = R(m_index->get_segment_len());

        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (auto entry : m_index->get_entries()) {
            if (this->skip_entry(query_len, series_len, entry.m_subs_info)) continue;

            Real min_dist_squared =
                this->get_min_dist_squared(entry.m_mts_summary, query_paa, distance_measure) * segment_len_r;
            pq.push({min_dist_squared, entry.m_subs_info});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

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
        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        auto &logger = QueryLogger::get_instance();

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        for (auto entry : m_index->get_entries()) {
            if (this->skip_entry(query_len, series_len, entry.m_subs_info)) continue;

            Real min_dist_squared = this->get_min_dist_squared(entry.m_mts_summary, query_paa, distance_measure);
            if (min_dist_squared >= result_set.get_distance_lb()) continue;

            this->update_result_set(entry.m_subs_info, m_index->get_pos_per_env(), query, query_len, result_set,
                                    distance_measure, dataset_ifs, real_query_inds);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        return {result_set.get_results(), true};
    }

    uptr<FinalizedFlatEnvelopeIndex> m_index;
    bool m_use_priority_queue;
};

#endif  // FLAT_ENVELOPE_INDEX_HPP
