#ifndef ISAX_FINALIZED_INDEX_HPP
#define ISAX_FINALIZED_INDEX_HPP

#include <fstream>
#include <queue>

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Paa.hpp"

/** @brief Properties of time series for iSAX indexes */
struct SeriesISaxProperties {
    /** @brief Length of the segments */
    uint m_segment_len;
    /** @brief Length of the time series in the dataset */
    uint m_series_len;
    /** @brief Number of channels of each series */
    MtsNumChannelsT m_num_channels;
    /** @brief Number of segments per channel */
    SaxSegIndT m_num_seg_per_channel;

    virtual ~SeriesISaxProperties() = default;

    SeriesISaxProperties(uint segment_len, uint series_len, MtsNumChannelsT num_channels,
                         SaxSegIndT num_seg_per_channel);

    SeriesISaxProperties() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(m_segment_len, m_series_len, m_num_channels, m_num_seg_per_channel);
    }
};

/** @brief Properties of time series for iSAX envelope indexes */
struct SeriesISaxEnvelopeProperties : SeriesISaxProperties {
    /** @brief Size of starting position groups */
    uint m_pos_per_env;

    SeriesISaxEnvelopeProperties(uint segment_len, uint series_len, MtsNumChannelsT num_channels,
                                 SaxSegIndT num_seg_per_channel, uint pos_per_env);

    SeriesISaxEnvelopeProperties() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<SeriesISaxProperties>(this), m_pos_per_env);
    }
};

/**
 * @brief Priority queue entry, intended to be used in iSaxFinalizedIndex
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
struct PQueueISaxEntry {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;

    Real m_min_dist_squared;
    vec<iSaxType> m_isax_words;
    const iSaxFinalizedNode<FTag>* m_node;

    bool operator<(const PQueueISaxEntry& other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

/**
 * @brief Finalized iSAX index
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
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
                       SaxNumBitsT alphabet_num_bits, vec<Real> breakpoints)
        : m_series_isax_prop(std::move(series_isax_prop)),
          m_first_layer_symbols(std::move(first_layer_symbols)),
          m_first_layer_nodes(std::move(first_layer_nodes)),
          m_first_layer_num_bits(first_layer_num_bits),
          m_alphabet_num_bits(alphabet_num_bits),
          m_breakpoints(std::move(breakpoints)) {
        assert(m_series_isax_prop->m_segment_len > 0);
        assert(m_first_layer_symbols.size() > 0);
    }

    ~iSaxFinalizedIndex() = default;

    std::pair<Real, Real> get_segment_limits(SaxNumBitsT num_bits, SymbolType symbol) const {
        uint num_shift = m_alphabet_num_bits - num_bits;
        auto [lower_ind, upper_ind] = get_limit_breakpoint_indexes(symbol, num_shift);
        return {
            lower_ind == -1 ? -INF : m_breakpoints[static_cast<uint>(lower_ind)],
            upper_ind == m_breakpoints.size() ? INF : m_breakpoints[static_cast<uint>(upper_ind)],
        };
    }

    std::pair<vec<iSaxType>, vec<iSaxType>> get_children_isax_words(const iSaxFinalizedNode<FTag>* node,
                                                                    vec<iSaxType> isax_words, MtsNumChannelsT c,
                                                                    SaxSegIndT s) const;

    const vec<vec<vec<SymbolType>>>& get_first_layer_symbols() const { return m_first_layer_symbols; }

    SaxNumBitsT get_first_layer_num_bits() const { return m_first_layer_num_bits; }

    const SeriesISaxProperties* get_series_isax_prop() const { return m_series_isax_prop.get(); }

    const iSaxFinalizedNode<FTag>* get_first_layer_node(size_t ind) const { return m_first_layer_nodes[ind].get(); }

   private:
    vec<vec<vec<SymbolType>>> m_first_layer_symbols;
    vec<uptr<iSaxFinalizedNode<FTag>>> m_first_layer_nodes;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    vec<Real> m_breakpoints;
    uptr<SeriesISaxProperties> m_series_isax_prop;

    std::pair<int, int> get_limit_breakpoint_indexes(SymbolType symbol, uint num_shift) const;

    MAKE_SERIALIZABLE((m_series_isax_prop, m_first_layer_symbols, m_first_layer_nodes, m_first_layer_num_bits,
                       m_alphabet_num_bits, m_breakpoints));
};

/**
 * @brief iSAX index search method
 * @tparam FTag The traits of the entries in the index
 * @tparam S The search type
 * @tparam D The distance type
 * @tparam QS Whether to sort the query or not
 */
template <typename FTag, SearchType S, DistanceType D, bool QS = false>
    requires ValidEntryTraitsTag<FTag>
class iSaxIndexSearch : public ISearchMethod<S, D, QS> {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

   public:
    /**
     * @brief Construct a new iSaxIndexSearch object
     * @param index The iSAX index to use for searching
     */
    iSaxIndexSearch(uptr<iSaxFinalizedIndex<FTag>> index) : m_index(std::move(index)) {}

    SearchResults search(const vec<vec<Real>>& query, const SearchOptions& opts, ResultSet<S>& result_set,
                         const DistanceMeasure<S, D, QS>& distance_measure, std::ifstream& dataset_ifs,
                         const vec<uint>* real_query_inds) const override {
        auto* series_isax_prop = m_index->get_series_isax_prop();
        uint series_len = series_isax_prop->m_series_len;
        uint segment_len = series_isax_prop->m_segment_len;
        Real segment_len_r = R(segment_len);
        MtsNumChannelsT num_channels = series_isax_prop->m_num_channels;
        SaxNumBitsT first_layer_num_bits = m_index->get_first_layer_num_bits();
        auto& first_layer_symbols = m_index->get_first_layer_symbols();

        assert(query.size() == num_channels);

        auto& logger = QueryLogger::get_instance();

        std::priority_queue<PQueueISaxEntry<FTag>> pq;

        auto [query_paa, query_len] = this->get_query_paa_and_len(query, segment_len, real_query_inds);

        // Go over first layer, calculate MINDIST and iSAX words, push to priority queue
        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (size_t i = 0; i < first_layer_symbols.size(); ++i) {
            Real min_dist_squared = 0;
            vec<iSaxType> isax_words(num_channels);

            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                for (SaxSegIndT s = 0; s < query_paa[c].size(); ++s) {
                    auto [lower, upper] =
                        m_index->get_segment_limits(first_layer_num_bits, first_layer_symbols[i][c][s]);
                    min_dist_squared += distance_measure.min_dist_squared(query_paa[c][s], lower, upper);
                }
                isax_words[c] = iSaxType(first_layer_symbols[i][c], first_layer_num_bits);
            }
            pq.push({min_dist_squared * segment_len_r, isax_words, m_index->get_first_layer_node(i)});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.start_timer(QC::TREE_TRAVERSAL_TIME_S);
        size_t leaves_visited = 0;
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
                    auto limits = m_index->get_segment_limits(num_bits, isax_words[c].symbol_no_shift(s));
                    Real prev_dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    ++num_bits;

                    auto [left_isax_words, right_isax_words] = m_index->get_children_isax_words(node, isax_words, c, s);

                    // Left child
                    limits = m_index->get_segment_limits(num_bits, left_isax_words[c].symbol_no_shift(s));
                    Real dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len_r * (dist - prev_dist), left_isax_words, left});

                    // Right child
                    limits = m_index->get_segment_limits(num_bits, right_isax_words[c].symbol_no_shift(s));
                    dist = distance_measure.min_dist_squared(query_paa[c][s], limits.first, limits.second);
                    pq.push({min_dist_squared + segment_len_r * (dist - prev_dist), right_isax_words, right});
                }
            } else {
                bool updated = false;

                vec<SubsequenceInfo> subsequence_infos = node->get_subsequence_infos();
                for (SubsequenceInfo subs_info : subsequence_infos) {
                    if (skip_entry(query_len, series_len, subs_info)) continue;

                    size_t data_to_read = subs_info.m_length;
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
                }
                logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED, static_cast<uint>(subsequence_infos.size()));
                logger.increment_count_col(QC::NUM_LEAVES_VISITED);

                if (!opts.m_exact && (++leaves_visited >= opts.m_max_leaves_to_visit || !updated)) break;
            }
            logger.increment_count_col(QC::NUM_NODES_VISITED);
        }
        logger.stop_timer(QC::TREE_TRAVERSAL_TIME_S);

        exact_results_found |= pq.empty();
        return {result_set.get_results(), exact_results_found};
    }

   private:
    uptr<iSaxFinalizedIndex<FTag>> m_index;

    bool skip_entry(uint query_len, uint series_len, const SubsequenceInfo& subs_info) const {
        if constexpr (std::is_same_v<FTag, PaaTag>) {
            return subs_info.m_length != query_len;
        } else if constexpr (std::is_same_v<FTag, EnvelopeTag>) {
            return series_len - subs_info.m_start_pos < query_len;
        }
        return false;
    }
};

#endif  // ISAX_FINALIZED_INDEX_HPP
