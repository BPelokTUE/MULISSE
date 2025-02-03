#include <fstream>
#include <queue>

#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"
#include "Summarization/Paa.hpp"

iSaxEnvelopeFinalizedIndex::iSaxEnvelopeFinalizedIndex(const SeriesISaxProperties& series_isax_prop,
                                                       vec<vec<iSaxWord>> first_isax_mins,
                                                       vec<vec<iSaxWord>> first_isax_maxs,
                                                       vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                                                       SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits,
                                                       vec<float> breakpoints, const str& dataset_path)
    : m_segment_len(series_isax_prop.segment_len),
      m_series_len(series_isax_prop.series_len),
      m_pos_per_env(series_isax_prop.pos_per_env),
      m_first_layer_nodes(std::move(first_layer_nodes)),
      m_first_layer_num_bits(first_layer_num_bits),
      m_alphabet_num_bits(alphabet_num_bits),
      m_num_seg_per_channel(first_isax_mins[0][0].size()),
      m_num_channels(first_isax_mins[0].size()),
      m_breakpoints(std::move(breakpoints)),
      m_dataset_path(dataset_path) {
    assert(m_segment_len > 0);

    size_t size_first_layer = first_isax_mins.size();

    m_first_sax_mins = vec<vec<vec<SaxSymbolT>>>(
        size_first_layer, vec<vec<SaxSymbolT>>(m_num_channels, vec<SaxSymbolT>(m_num_seg_per_channel)));

    m_first_sax_maxs = vec<vec<vec<SaxSymbolT>>>(
        size_first_layer, vec<vec<SaxSymbolT>>(m_num_channels, vec<SaxSymbolT>(m_num_seg_per_channel)));

    for (size_t i = 0; i < size_first_layer; ++i) {
        for (size_t j = 0; j < m_num_channels; ++j) {
            m_first_sax_mins[i][j] = first_isax_mins[i][j].get_symbols_no_shift();
            m_first_sax_maxs[i][j] = first_isax_maxs[i][j].get_symbols_no_shift();
        }
    }
}

std::pair<float, float> iSaxEnvelopeFinalizedIndex::get_segment_limits(SaxNumBitsT num_bits, SaxSymbolT min_symbol,
                                                                       SaxSymbolT max_symbol) const {
    unsigned num_shift = m_alphabet_num_bits - num_bits;
    int lower_ind = (min_symbol << num_shift) - 1, upper_ind = ((max_symbol + 1) << num_shift) - 1;
    return {
        lower_ind == -1 ? NEG_INF : m_breakpoints[lower_ind],
        upper_ind == m_breakpoints.size() ? INF : m_breakpoints[upper_ind],
    };
}
struct PQueueEntry {
    DistanceT min_dist_squared;
    vec<iSaxWord> isax_mins, isax_maxs;
    const iSaxFinalizedNode* node;

    bool operator<(const PQueueEntry& other) const { return min_dist_squared > other.min_dist_squared; }
};

vec<SearchResult> iSaxEnvelopeFinalizedIndex::search(const vec<vec<float>>& query, const SearchOptions& opts) const {
    assert(query.size() == m_num_channels);

    std::ifstream data_stream(m_dataset_path, std::ios::binary);

    std::priority_queue<PQueueEntry> pq;

    vec<vec<float>> query_paa(m_num_channels);
    for (size_t i = 0; i < m_num_channels; ++i) query_paa[i] = paa(query[i], m_segment_len);

    IDistanceMeasure* distance_measure = opts.distance_measure.get();
    IResultSet* result_set = opts.result_set.get();

    // Go over first layer, calculate MINDIST and iSAX words, push to priority queue
    for (size_t i = 0; i < m_first_sax_mins.size(); ++i) {
        DistanceT min_dist_squared = 0;
        vec<iSaxWord> isax_mins(m_num_channels), isax_maxs(m_num_channels);

        for (size_t c = 0; c < m_num_channels; ++c) {
            for (size_t s = 0; s < query_paa[c].size(); ++s) {
                auto [lower, upper] =
                    get_segment_limits(m_first_layer_num_bits, m_first_sax_mins[i][c][s], m_first_sax_maxs[i][c][s]);
                min_dist_squared += distance_measure->min_dist_squared(query_paa[c][s], lower, upper);
            }
            isax_mins[c] = iSaxWord(m_first_sax_mins[i][c], m_first_layer_num_bits);
            isax_maxs[c] = iSaxWord(m_first_sax_maxs[i][c], m_first_layer_num_bits);
        }
        pq.push({min_dist_squared, isax_mins, isax_maxs, m_first_layer_nodes[i].get()});
    }

    while (!pq.empty()) {
        auto [min_dist_squared, isax_mins, isax_maxs, node] = pq.top();
        pq.pop();

        if (min_dist_squared > result_set->get_distance_lb()) break;

        if (!(node->is_leaf())) {
            auto [c, s] = node->get_split_ind();
            unsigned num_bits = isax_mins[c].get_num_bits()[s];
            auto limits = get_segment_limits(num_bits, isax_mins[c][s], isax_maxs[c][s]);
            auto [left, right] = node->get_children();
            auto [max_symbol_left, max_symbol_right] = node->get_children_max_symbols();
            float prev_dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
            ++num_bits;

            // Left child
            vec<iSaxWord> left_isax_mins = isax_mins, left_isax_maxs = isax_maxs;
            left_isax_mins[c].append_to_symbol(s, 0);
            left_isax_maxs[c].set_symbol(s, num_bits, max_symbol_left);
            limits = get_segment_limits(num_bits, left_isax_mins[c][s], left_isax_maxs[c][s]);
            float dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
            pq.push({min_dist_squared - prev_dist + dist, left_isax_mins, left_isax_maxs, left});

            // Right child
            isax_mins[c].append_to_symbol(s, 1);
            isax_maxs[c].set_symbol(s, num_bits, max_symbol_right);
            limits = get_segment_limits(num_bits, isax_mins[c][s], isax_maxs[c][s]);
            dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
            pq.push({min_dist_squared - prev_dist + dist, isax_mins, isax_maxs, right});
        } else {
            vec<FilePositionT> file_positions = node->get_file_positions();
            for (FilePositionT file_pos : file_positions) {
                size_t data_remaining = m_series_len - (file_pos % m_series_len);
                size_t data_to_read = std::min(query[0].size() + m_pos_per_env - 1, data_remaining);

                vec<vec<float>> subsequence(m_num_channels, vec<float>(data_to_read));
                for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
                    data_stream.seekg(file_pos + c * m_series_len * sizeof(float));
                    data_stream.read(reinterpret_cast<char*>(subsequence[c].data()), data_to_read * sizeof(float));
                }
                distance_measure->update_result_set(result_set, file_pos, query, subsequence);
            }
        }
    }

    return result_set->get_results();
};
