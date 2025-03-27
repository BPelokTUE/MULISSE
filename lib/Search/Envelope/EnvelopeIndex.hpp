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
     * @param m_segment_len Length of the segments
     * @param m_pos_per_env Number of positions per envelope
     */
    FlatEnvelopeIndex(uint m_segment_len, uint m_pos_per_env)
        : m_segment_len(m_segment_len), m_pos_per_env(m_pos_per_env) {}

    FlatEnvelopeIndex() = default;

    void insert(const IndexEntry<Envelope> &entry) override { m_entries.push_back(entry); }

    void insert_entries(const vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override {
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

    uint get_segment_len() { return m_segment_len; }

    const vec<IndexEntry<Envelope>> &get_entries() const { return m_entries; }

   private:
    vec<IndexEntry<Envelope>> m_entries;
    uint m_segment_len, m_pos_per_env;

    MAKE_SERIALIZABLE((m_segment_len, m_pos_per_env, m_entries));
};

template <SearchType S, DistanceType D>
class EnvelopeIndexSearch : public ISearchMethod<S, D> {
   public:
    EnvelopeIndexSearch(uptr<FlatEnvelopeIndex> index) : m_index(std::move(index)) {}

    vec<SearchResult> search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                             const DistanceMeasure<S, D> &distance_measure, std::ifstream &dataset_ifs) const override {
        auto &RS = RunSettings::get_instance();
        auto &logger = QueryLogger::get_instance();
        uint segment_len = m_index->get_segment_len();

        uint series_len = RS.get_dataset_props().series_len;
        MtsNumChannelsT num_channels = RS.get_dataset_props().num_channels;

        vec<vec<Real>> query_paa(num_channels);
        size_t query_len = 0;
        for (size_t c = 0; c < num_channels; ++c) {
            query_paa[c] = paa(query[c], segment_len);
            query_len = std::max(query_len, query[c].size());
        }

        std::priority_queue<PQueueEnvelopeEntry> pq;

        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (auto entry : m_index->get_entries()) {
            if (entry.subsequence_info.length < query_len) continue;

            Real min_dist_squared = 0;
            for (MtsNumChannelsT c = 0; c < num_channels; ++c)
                for (uint s = 0; s < query_paa[c].size(); ++s)
                    min_dist_squared += distance_measure.min_dist_squared(
                        query_paa[c][s], entry.mts_summary[c].lower[s], entry.mts_summary[c].upper[s]);

            pq.push({min_dist_squared * segment_len, entry.subsequence_info});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        while (!pq.empty()) {
            auto [min_dist_squared, subs_info] = pq.top();
            pq.pop();

            if (min_dist_squared >= result_set.get_distance_lb()) break;

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
            distance_measure.update_result_set(result_set, subs_info, query, subsequence);
            logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

            logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        return result_set.get_results();
    };

   private:
    uptr<FlatEnvelopeIndex> m_index;
};

#endif  // ENVELOPE_INDEX_HPP
