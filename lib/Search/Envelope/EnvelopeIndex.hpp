#ifndef ENVELOPE_INDEX_HPP
#define ENVELOPE_INDEX_HPP

#include <queue>

#include "Search/Index.hpp"
#include "Search/TopDownInserter.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Util/typedefs.hpp"

struct PQueueEnvelopeEntry {
    Real min_dist_squared;
    SubsequenceInfo subs_info;

    bool operator<(const PQueueEnvelopeEntry &other) const { return min_dist_squared > other.min_dist_squared; }
};

/** @brief Flat envelope index */
class FlatEnvelopeIndex : public IIndex<Envelope>,
                          public IFinalizedIndex<EnvelopeTag>,
                          public std::enable_shared_from_this<FlatEnvelopeIndex> {
   public:
    /**
     * @brief Construct a new FlatEnvelopeIndex instance
     * @param segment_len Length of the segments
     * @param pos_per_env Number of positions per envelope
     * @param num_bits Number of bits used for SAX discretization. Defaults to 0, indicating no discretization.
     * If greater than 0, the breakpoints are assumed to have the same cardinality.
     */
    FlatEnvelopeIndex(uint segment_len, uint pos_per_env, SaxNumBitsT num_bits = 0)
        : m_segment_len(segment_len), m_pos_per_env(pos_per_env), m_num_bits(num_bits) {}

    FlatEnvelopeIndex() = default;

    void insert(IndexEntry<Envelope> &entry) override {
        if (m_num_bits > 0) {
            auto &breakpoints = RunSettings::get_instance().get_breakpoints();
            SaxSegIndT num_seg_per_channel = entry.mts_summary[0].lower.size();

            for (MtsNumChannelsT c = 0; c < entry.mts_summary.size(); ++c) {
                SaxWord sax_lower(entry.mts_summary[c].lower, m_num_bits, breakpoints);
                SaxWord sax_upper(entry.mts_summary[c].upper, m_num_bits, breakpoints);

                for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) {
                    entry.mts_summary[c].lower[s] = sax_lower[s] > 0 ? breakpoints[sax_lower[s] - 1] : -INF;
                    entry.mts_summary[c].upper[s] = sax_upper[s] < breakpoints.size() ? breakpoints[sax_upper[s]] : INF;
                }
            }
        }
        m_entries.push_back(entry);
    }

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override {
        uptr<IEntryInserter<FlatEnvelopeIndex>> inserter;
        switch (inserter_type) {
            case TOP_DOWN:
                inserter = std::make_unique<TopDownInserter<FlatEnvelopeIndex>>(this->shared_from_this());
                break;
            default:
                throw std::invalid_argument("Invalid inserter type");
        }
        inserter->insert_entries(entries);
    };

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override {
        auto finalized = new FlatEnvelopeIndex(m_segment_len, m_pos_per_env);
        finalized->m_entries = std::move(m_entries);
        return uptr<IFinalizedIndex<EnvelopeTag>>(finalized);
    }

    inline uint get_segment_len() { return m_segment_len; }

    const vec<IndexEntry<Envelope>> &get_entries() const { return m_entries; }

   private:
    vec<IndexEntry<Envelope>> m_entries;
    uint m_segment_len, m_pos_per_env;
    SaxNumBitsT m_num_bits;

    MAKE_SERIALIZABLE((m_segment_len, m_pos_per_env, m_entries));
};

/**
 * @brief Flat envelope index search method
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 */
template <SearchType S, DistanceType D, bool QS = false>
class FlatEnvelopeIndexSearch : public ISearchMethod<S, D, QS> {
   public:
    FlatEnvelopeIndexSearch(uptr<FlatEnvelopeIndex> index, bool use_priority_queue = true)
        : m_index(std::move(index)), m_use_priority_queue(use_priority_queue) {}

    vec<SearchResult> search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
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
    inline vec<SearchResult> search_with_priority_queue(const vec<vec<Real>> &query, const vec<vec<Real>> &query_paa,
                                                        uint query_len, ResultSet<S> &result_set,
                                                        const DistanceMeasure<S, D, QS> &distance_measure,
                                                        std::ifstream &dataset_ifs,
                                                        const vec<uint> *real_query_inds) const {
        auto &logger = QueryLogger::get_instance();

        std::priority_queue<PQueueEnvelopeEntry> pq;

        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (auto entry : m_index->get_entries()) {
            if (entry.subsequence_info.length < query_len) continue;

            Real min_dist_squared = get_min_dist_squared(entry, query_paa, result_set, distance_measure);
            pq.push({min_dist_squared * m_index->get_segment_len(), entry.subsequence_info});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        while (!pq.empty()) {
            auto [min_dist_squared, subs_info] = pq.top();
            pq.pop();

            if (min_dist_squared >= result_set.get_distance_lb()) break;
            update_result_set(subs_info, query, result_set, distance_measure, dataset_ifs, real_query_inds);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        return result_set.get_results();
    }

    inline vec<SearchResult> search_sequentially(const vec<vec<Real>> &query, const vec<vec<Real>> &query_paa,
                                                 uint query_len, ResultSet<S> &result_set,
                                                 const DistanceMeasure<S, D, QS> &distance_measure,
                                                 std::ifstream &dataset_ifs, const vec<uint> *real_query_inds) const {
        auto &logger = QueryLogger::get_instance();

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        for (auto entry : m_index->get_entries()) {
            if (entry.subsequence_info.length < query_len) continue;

            Real min_dist_squared = get_min_dist_squared(entry, query_paa, result_set, distance_measure);
            if (min_dist_squared >= result_set.get_distance_lb()) continue;

            update_result_set(entry.subsequence_info, query, result_set, distance_measure, dataset_ifs,
                              real_query_inds);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        return result_set.get_results();
    }

    inline Real get_min_dist_squared(const IndexEntry<Envelope> &entry, const vec<vec<Real>> &query_paa,
                                     ResultSet<S> &result_set,
                                     const DistanceMeasure<S, D, QS> &distance_measure) const {
        Real min_dist_squared = 0;
        for (MtsNumChannelsT c = 0; c < query_paa.size(); ++c)
            for (uint s = 0; s < query_paa[c].size(); ++s)
                min_dist_squared += distance_measure.min_dist_squared(query_paa[c][s], entry.mts_summary[c].lower[s],
                                                                      entry.mts_summary[c].upper[s]);
        return min_dist_squared;
    }

    inline void update_result_set(const SubsequenceInfo &subs_info, const vec<vec<Real>> &query,
                                  ResultSet<S> &result_set, const DistanceMeasure<S, D, QS> &distance_measure,
                                  std::ifstream &dataset_ifs, const vec<uint> *real_query_inds) const {
        auto &logger = QueryLogger::get_instance();

        auto &RS = RunSettings::get_instance();
        uint series_len = RS.get_dataset_props().series_len;
        MtsNumChannelsT num_channels = RS.get_dataset_props().num_channels;

        vec<vec<Real>> subsequence(num_channels);
        logger.start_timer(QC::IO_TIME_S);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (query[c].empty()) continue;

            subsequence[c].resize(subs_info.length);
            dataset_ifs.seekg(subs_info.get_file_pos(series_len, num_channels, c));
            dataset_ifs.read(reinterpret_cast<char *>(subsequence[c].data()), subs_info.length * sizeof(Real));
        }
        logger.stop_timer(QC::IO_TIME_S);

        logger.start_timer(QC::TS_EXAMINATION_TIME_S);
        distance_measure.update_result_set(result_set, subs_info, query, subsequence, real_query_inds);
        logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

        logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED);
    }

    uptr<FlatEnvelopeIndex> m_index;
    bool m_use_priority_queue;
};

#endif  // ENVELOPE_INDEX_HPP
