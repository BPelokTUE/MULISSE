#ifndef ISAX_FINALIZED_INDEX_HPP
#define ISAX_FINALIZED_INDEX_HPP

#include <fstream>
#include <queue>

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"

/** @brief Properties of time series for iSAX indexes */
struct SeriesISaxProperties {
    /** @brief Length of the segments */
    uint segment_len;
    /** @brief Length of the time series in the dataset */
    uint series_len;
    /** @brief Number of channels of each series */
    MtsNumChannelsT num_channels;
    /** @brief Number of segments per channel */
    SaxSegIndT num_seg_per_channel;

    virtual ~SeriesISaxProperties() = default;

    SeriesISaxProperties(uint segment_len, uint series_len, MtsNumChannelsT num_channels,
                         SaxSegIndT num_seg_per_channel);

    SeriesISaxProperties() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(segment_len, series_len, num_channels, num_seg_per_channel);
    }
};

/** @brief Properties of time series for iSAX envelope indexes */
struct SeriesISaxEnvelopeProperties : SeriesISaxProperties {
    /** @brief Size of starting position groups */
    uint pos_per_env;

    SeriesISaxEnvelopeProperties(uint segment_len, uint series_len, MtsNumChannelsT num_channels,
                                 SaxSegIndT num_seg_per_channel, uint pos_per_env);

    SeriesISaxEnvelopeProperties() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<SeriesISaxProperties>(this), pos_per_env);
    }
};

CEREAL_REGISTER_TYPE(SeriesISaxProperties)
CEREAL_REGISTER_TYPE(SeriesISaxEnvelopeProperties)
CEREAL_REGISTER_POLYMORPHIC_RELATION(SeriesISaxProperties, SeriesISaxEnvelopeProperties)

template <typename FTag>
    requires ValidSaxTraitsTag<FTag>
struct PQueueEntry {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;

    DistanceT min_dist_squared;
    vec<iSaxType> isax_words;
    const iSaxFinalizedNode<FTag>* node;

    bool operator<(const PQueueEntry& other) const { return min_dist_squared > other.min_dist_squared; }
};

template <typename FTag>
    requires ValidSaxTraitsTag<FTag>
class iSaxFinalizedIndex : public IFinalizedIndex<FTag> {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

   public:
    iSaxFinalizedIndex() = default;

    /**
     * @brief Construct a new iSaxFinalizedIndex object
     *
     * @param series_isax_prop Properties of the time series
     * @param first_layer_symbols Symbols of the first layer
     * @param first_layer_nodes First layer nodes
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param alphabet_num_bits The maximum number of bits used for any symbol in any node of the index
     * @param breakpoints Breakpoints used for the iSAX index; assumed to be `2^alphabet_num_bits-1` long;
     *        does not include `-inf` and `inf`
     */
    iSaxFinalizedIndex(uptr<SeriesISaxProperties> series_isax_prop, vec<vec<vec<SymbolType>>> first_layer_symbols,
                       vec<uptr<iSaxFinalizedNode<FTag>>> first_layer_nodes, SaxNumBitsT first_layer_num_bits,
                       SaxNumBitsT alphabet_num_bits, vec<float> breakpoints)
        : m_series_isax_prop(std::move(series_isax_prop)),
          m_first_layer_symbols(std::move(first_layer_symbols)),
          m_first_layer_nodes(std::move(first_layer_nodes)),
          m_first_layer_num_bits(first_layer_num_bits),
          m_alphabet_num_bits(alphabet_num_bits),
          m_breakpoints(std::move(breakpoints)) {
        assert(m_series_isax_prop->segment_len > 0);
        assert(m_first_layer_symbols.size() > 0);
    }

    ~iSaxFinalizedIndex() = default;

    std::pair<float, float> get_segment_limits(SaxNumBitsT num_bits, SymbolType symbol) const;

    std::pair<vec<iSaxType>, vec<iSaxType>> get_children_isax_words(const iSaxFinalizedNode<FTag>* node,
                                                                    vec<iSaxType> isax_words, MtsNumChannelsT c,
                                                                    SaxSegIndT s) const;

    vec<SearchResult> search(const vec<vec<float>>& query, const SearchOptions& opts,
                             std::ifstream& dataset_ifs) const override {
        uint series_len = m_series_isax_prop->series_len;
        uint segment_len = m_series_isax_prop->segment_len;
        MtsNumChannelsT num_channels = m_series_isax_prop->num_channels;

        assert(query.size() == num_channels);

        auto& logger = QueryLogger::get_instance();

        std::priority_queue<PQueueEntry<FTag>> pq;

        vec<vec<float>> query_paa(num_channels);
        size_t query_len = 0;
        for (size_t c = 0; c < num_channels; ++c) {
            query_paa[c] = paa(query[c], segment_len);
            query_len = std::max(query_len, query[c].size());
        }

        IDistanceMeasure* distance_measure = opts.distance_measure.get();
        IResultSet* result_set = opts.result_set.get();

        // Go over first layer, calculate MINDIST and iSAX words, push to priority queue
        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (size_t i = 0; i < m_first_layer_symbols.size(); ++i) {
            DistanceT min_dist_squared = 0;
            vec<iSaxType> isax_words(num_channels);

            for (size_t c = 0; c < num_channels; ++c) {
                for (size_t s = 0; s < query_paa[c].size(); ++s) {
                    auto [lower, upper] = get_segment_limits(m_first_layer_num_bits, m_first_layer_symbols[i][c][s]);
                    min_dist_squared += distance_measure->min_dist_squared(query_paa[c][s], lower, upper);
                }
                isax_words[c] = iSaxType(m_first_layer_symbols[i][c], m_first_layer_num_bits);
            }
            pq.push({min_dist_squared * segment_len, isax_words, m_first_layer_nodes[i].get()});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        while (!pq.empty()) {
            auto [min_dist_squared, isax_words, node] = pq.top();
            pq.pop();

            if (min_dist_squared >= result_set->get_distance_lb()) break;

            if (!(node->is_leaf())) {
                auto [s, c] = node->get_split_ind();
                auto [left, right] = node->get_children();

                if (query[c].empty() || query_paa[c].size() <= s) {
                    pq.push({min_dist_squared, isax_words, left});
                    pq.push({min_dist_squared, isax_words, right});
                } else {
                    uint num_bits = isax_words[c].get_num_bits()[s];
                    auto limits = get_segment_limits(num_bits, isax_words[c].symbol_no_shift(s));
                    float prev_dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    ++num_bits;

                    auto [left_isax_words, right_isax_words] = get_children_isax_words(node, isax_words, c, s);

                    // Left child
                    limits = get_segment_limits(num_bits, left_isax_words[c].symbol_no_shift(s));
                    float dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len * (dist - prev_dist), left_isax_words, left});

                    // Right child
                    limits = get_segment_limits(num_bits, right_isax_words[c].symbol_no_shift(s));
                    dist = distance_measure->min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len * (dist - prev_dist), right_isax_words, right});
                }
            } else {
                vec<SubsequenceInfo> subsequence_positions = node->get_subsequence_positions();
                for (SubsequenceInfo subs_info : subsequence_positions) {
                    size_t data_remaining = series_len - subs_info.start_pos;

                    if (data_remaining < query_len) continue;

                    size_t data_to_read = subs_info.length;
                    vec<vec<float>> subsequence(num_channels);
                    logger.start_timer(QC::IO_TIME_S);
                    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                        if (query[c].empty()) continue;

                        subsequence[c].resize(data_to_read);
                        dataset_ifs.seekg(subs_info.get_file_pos(series_len, num_channels, c));
                        dataset_ifs.read(reinterpret_cast<char*>(subsequence[c].data()), data_to_read * sizeof(float));
                    }
                    logger.stop_timer(QC::IO_TIME_S);

                    logger.start_timer(QC::TS_EXAMINATION_TIME_S);
                    distance_measure->update_result_set(result_set, subs_info, query, subsequence);
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
    }

    const vec<vec<vec<SymbolType>>>& get_first_layer_symbols() const { return m_first_layer_symbols; }

    SaxNumBitsT get_first_layer_num_bits() const { return m_first_layer_num_bits; }

   private:
    vec<vec<vec<SymbolType>>> m_first_layer_symbols;
    vec<uptr<iSaxFinalizedNode<FTag>>> m_first_layer_nodes;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    vec<float> m_breakpoints;
    uptr<SeriesISaxProperties> m_series_isax_prop;

    MAKE_SERIALIZABLE((m_series_isax_prop, m_first_layer_symbols, m_first_layer_nodes, m_first_layer_num_bits,
                       m_alphabet_num_bits, m_breakpoints));
};

#endif  // ISAX_FINALIZED_INDEX_HPP
