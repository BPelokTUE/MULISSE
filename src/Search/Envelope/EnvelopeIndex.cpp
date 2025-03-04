#include "queue"

#include "Search/Envelope/EnvelopeIndex.hpp"

FlatEnvelopeIndex::FlatEnvelopeIndex(uint segment_len, uint pos_per_env)
    : m_segment_len(segment_len), m_pos_per_env(pos_per_env) {}

void FlatEnvelopeIndex::insert(const IndexEntry<Envelope> &entry) { m_entries.push_back(entry); }

uptr<IFinalizedIndex<EnvelopeTag>> FlatEnvelopeIndex::finalize() {
    auto finalized = new FlatEnvelopeIndex(m_segment_len, m_pos_per_env);
    finalized->m_entries = std::move(m_entries);
    return uptr<IFinalizedIndex<EnvelopeTag>>(finalized);
}

struct PQueueEnvelopeEntry {
    DistanceT min_dist_squared;
    SubsequenceInfo subs_info;

    bool operator<(const PQueueEnvelopeEntry &other) const { return min_dist_squared > other.min_dist_squared; }
};

vec<SearchResult> FlatEnvelopeIndex::search(const vec<vec<float>> &query, const SearchOptions &opts,
                                            std::ifstream &dataset_ifs) const {
    auto &RS = RunSettings::get_instance();
    auto &logger = QueryLogger::get_instance();

    uint series_len = RS.get_dataset_props().series_len;
    MtsNumChannelsT num_channels = RS.get_dataset_props().num_channels;
    IDistanceMeasure *distance_measure = opts.distance_measure.get();
    IResultSet *result_set = opts.result_set.get();

    vec<vec<float>> query_paa(num_channels);
    size_t query_len = 0;
    for (size_t c = 0; c < num_channels; ++c) {
        query_paa[c] = paa(query[c], m_segment_len);
        query_len = std::max(query_len, query[c].size());
    }

    std::priority_queue<PQueueEnvelopeEntry> pq;

    logger.start_timer(QC::FIRST_LAYER_TIME_S);
    for (auto entry : m_entries) {
        if (entry.subsequence_info.length < query_len) continue;

        DistanceT min_dist_squared = 0;
        for (MtsNumChannelsT c = 0; c < num_channels; ++c)
            for (uint s = 0; s < query_paa[c].size(); ++s)
                min_dist_squared += distance_measure->min_dist_squared(query_paa[c][s], entry.mts_summary[c].lower[s],
                                                                       entry.mts_summary[c].upper[s]);

        pq.push({min_dist_squared * m_segment_len, entry.subsequence_info});
    }
    logger.stop_timer(QC::FIRST_LAYER_TIME_S);

    logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
    while (!pq.empty()) {
        auto [min_dist_squared, subs_info] = pq.top();
        pq.pop();

        if (min_dist_squared >= opts.result_set->get_distance_lb()) break;

        vec<vec<float>> subsequence(num_channels);
        logger.start_timer(QC::IO_TIME_S);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (query[c].empty()) continue;

            subsequence[c].resize(subs_info.length);
            dataset_ifs.seekg(subs_info.get_file_pos(series_len, num_channels, c));
            dataset_ifs.read(reinterpret_cast<char *>(subsequence[c].data()), subs_info.length * sizeof(float));
        }
        logger.stop_timer(QC::IO_TIME_S);

        logger.start_timer(QC::TS_EXAMINATION_TIME_S);
        distance_measure->update_result_set(result_set, subs_info, query, subsequence);
        logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

        logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED);
    }
    logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

    return result_set->get_results();
}

const vec<IndexEntry<Envelope>> &FlatEnvelopeIndex::get_entries() const { return m_entries; }
