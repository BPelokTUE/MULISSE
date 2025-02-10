#include <fstream>
#include <queue>

#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"
#include "Search/ResultSet.hpp"
#include "Search/EnvelopeIndex.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"

iSaxEnvelopeFinalizedIndex::iSaxEnvelopeFinalizedIndex(const SeriesISaxProperties& series_isax_prop,
                                                       vec<vec<vec<SaxSymbolT>>> first_layer_min_symbols,
                                                       vec<vec<vec<SaxSymbolT>>> first_layer_max_symbols,
                                                       vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                                                       SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits,
                                                       vec<float> breakpoints)
    : m_segment_len(series_isax_prop.segment_len),
      m_first_layer_min_symbols(std::move(first_layer_min_symbols)),
      m_first_layer_max_symbols(std::move(first_layer_max_symbols)),
      m_first_layer_nodes(std::move(first_layer_nodes)),
      m_first_layer_num_bits(first_layer_num_bits),
      m_alphabet_num_bits(alphabet_num_bits),
      m_breakpoints(std::move(breakpoints)) {
    assert(m_segment_len > 0);
    assert(m_first_layer_min_symbols.size() > 0);
    assert(m_first_layer_min_symbols.size() == m_first_layer_max_symbols.size());

    m_num_seg_per_channel = m_first_layer_max_symbols[0][0].size();
    IEnvelopeFinalizedIndex::m_series_len = series_isax_prop.series_len;
    IEnvelopeFinalizedIndex::m_pos_per_env = series_isax_prop.pos_per_env;
    IEnvelopeFinalizedIndex::m_num_channels = series_isax_prop.num_channels;
}

std::pair<float, float> iSaxEnvelopeFinalizedIndex::get_segment_limits(SaxNumBitsT num_bits, SaxSymbolT min_symbol,
                                                                       SaxSymbolT max_symbol) const {
    uint num_shift = m_alphabet_num_bits - num_bits;
    int lower_ind = (min_symbol << num_shift) - 1, upper_ind = ((max_symbol + 1) << num_shift) - 1;
    return {
        lower_ind == -1 ? -INF : m_breakpoints[lower_ind],
        upper_ind == m_breakpoints.size() ? INF : m_breakpoints[upper_ind],
    };
}
struct PQueueEntry {
    DistanceT min_dist_squared;
    vec<iSaxWord> isax_mins, isax_maxs;
    const iSaxFinalizedNode* node;

    bool operator<(const PQueueEntry& other) const { return min_dist_squared > other.min_dist_squared; }
};

vec<SearchResult> iSaxEnvelopeFinalizedIndex::search(const vec<vec<float>>& query, const SearchOptions& opts,
                                                     std::ifstream& dataset_ifs) const {
    assert(query.size() == m_num_channels);

    auto& logger = QueryLogger::get_instance();

    std::priority_queue<PQueueEntry> pq;

    vec<vec<float>> query_paa(m_num_channels);
    size_t query_len = 0;
    for (size_t c = 0; c < m_num_channels; ++c) {
        query_paa[c] = paa(query[c], m_segment_len);
        query_len = std::max(query_len, query[c].size());
    }

    IDistanceMeasure* distance_measure = opts.distance_measure.get();
    IResultSet* result_set = opts.result_set.get();

    // Go over first layer, calculate MINDIST and iSAX words, push to priority queue
    logger.start_timer(QC::FIRST_LAYER_TIME_S);
    for (size_t i = 0; i < m_first_layer_min_symbols.size(); ++i) {
        DistanceT min_dist_squared = 0;
        vec<iSaxWord> isax_mins(m_num_channels), isax_maxs(m_num_channels);

        for (size_t c = 0; c < m_num_channels; ++c) {
            for (size_t s = 0; s < query_paa[c].size(); ++s) {
                auto [lower, upper] = get_segment_limits(m_first_layer_num_bits, m_first_layer_min_symbols[i][c][s],
                                                         m_first_layer_max_symbols[i][c][s]);
                min_dist_squared += distance_measure->min_dist_squared(query_paa[c][s], lower, upper);
            }
            isax_mins[c] = iSaxWord(m_first_layer_min_symbols[i][c], m_first_layer_num_bits);
            isax_maxs[c] = iSaxWord(m_first_layer_max_symbols[i][c], m_first_layer_num_bits);
        }
        pq.push({min_dist_squared * m_segment_len, isax_mins, isax_maxs, m_first_layer_nodes[i].get()});
    }
    logger.stop_timer(QC::FIRST_LAYER_TIME_S);

    logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
    while (!pq.empty()) {
        auto [min_dist_squared, isax_mins, isax_maxs, node] = pq.top();
        pq.pop();

        if (min_dist_squared > result_set->get_distance_lb()) break;

        if (!(node->is_leaf())) {
            auto [s, c] = node->get_split_ind();
            auto [left, right] = node->get_children();

            if (query[c].empty() || query_paa[c].size() <= s) {
                pq.push({min_dist_squared, isax_mins, isax_maxs, left});
                pq.push({min_dist_squared, isax_mins, isax_maxs, right});
            } else {
                uint num_bits = isax_mins[c].get_num_bits()[s];
                auto limits =
                    get_segment_limits(num_bits, isax_mins[c].symbol_no_shift(s), isax_maxs[c].symbol_no_shift(s));
                auto [max_symbol_left, max_symbol_right] =
                    node->get_children_max_symbols(num_bits, m_alphabet_num_bits);
                float prev_dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
                ++num_bits;

                // Left child
                vec<iSaxWord> left_isax_mins = isax_mins, left_isax_maxs = isax_maxs;
                left_isax_mins[c].append_to_symbol(s, 0);
                left_isax_maxs[c].set_symbol(s, num_bits, max_symbol_left);
                limits = get_segment_limits(num_bits, left_isax_mins[c].symbol_no_shift(s),
                                            left_isax_maxs[c].symbol_no_shift(s));
                float dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
                pq.push({min_dist_squared + m_segment_len * (dist - prev_dist), left_isax_mins, left_isax_maxs, left});

                // Right child
                vec<iSaxWord> right_isax_mins = std::move(isax_mins), right_isax_maxs = std::move(isax_maxs);
                right_isax_mins[c].append_to_symbol(s, 1);
                right_isax_maxs[c].set_symbol(s, num_bits, max_symbol_right);
                limits = get_segment_limits(num_bits, right_isax_mins[c].symbol_no_shift(s),
                                            right_isax_maxs[c].symbol_no_shift(s));
                dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
                pq.push(
                    {min_dist_squared + m_segment_len * (dist - prev_dist), right_isax_mins, right_isax_maxs, right});
            }
        } else {
            vec<FilePositionT> file_positions = node->get_file_positions();
            for (FilePositionT file_pos : file_positions) {
                size_t data_remaining = m_series_len - (file_pos % m_series_len);

                if (data_remaining < query_len) continue;

                size_t data_to_read = std::min(query_len + IEnvelopeFinalizedIndex::m_pos_per_env - 1, data_remaining);
                vec<vec<float>> subsequence(m_num_channels);
                logger.start_timer(QC::IO_TIME_S);
                for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
                    if (query[c].empty()) continue;

                    subsequence[c].resize(data_to_read);
                    FilePositionT start_byte = (file_pos + c * m_series_len) * sizeof(float);
                    dataset_ifs.seekg(start_byte);
                    dataset_ifs.read(reinterpret_cast<char*>(subsequence[c].data()), data_to_read * sizeof(float));
                }
                logger.stop_timer(QC::IO_TIME_S);

                logger.start_timer(QC::TS_EXAMINATION_TIME_S);
                distance_measure->update_result_set(result_set, file_pos, query, subsequence, true);
                logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

                // TODO: Discuss how pruning ratio should be calculated when envs_per_ts > 1
                logger.increment_count_col(QC::NUM_TS_EXAMINED);
            }
            logger.increment_count_col(QC::NUM_LEAVES_VISITED);
        }
        logger.increment_count_col(QC::NUM_NODES_VISITED);
    }
    logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

    return result_set->get_results();
};
