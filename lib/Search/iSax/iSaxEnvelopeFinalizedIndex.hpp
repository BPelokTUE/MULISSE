#ifndef I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
#define I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include "Search/EnvelopeIndex.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"

/** @brief Properties of time series for iSAX indexes */
struct SeriesISaxProperties {
    /** @brief Length of the segments */
    unsigned segment_len;
    /** @brief Length of the time series in the dataset */
    unsigned series_len;
    /** @brief Size of starting position groups */
    unsigned pos_per_env;
    /** @brief Number of channels of each series */
    MtsNumChannelsT num_channels;
    /** @brief Number of segments per channel */
    SaxSegIndT num_seg_per_channel;
};

/** @brief Finalized iSAX (ULISSE) index */
class iSaxEnvelopeFinalizedIndex : public IEnvelopeFinalizedIndex {
   public:
    iSaxEnvelopeFinalizedIndex() = default;

    /**
     * @brief Construct a new iSaxEnvelopeFinalizedIndex object
     *
     * @param series_isax_prop Properties of the time series
     * @param first_isax_mins iSAX min words of the first layer. Will be treated as SAX words.
     * @param first_isax_maxs iSAX max words of the first layer. Will be treated as SAX words.
     * @param first_layer_nodes First layer nodes
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param alphabet_num_bits The maximum number of bits used for any symbol in any node of the index
     * @param breakpoints Breakpoints used for the iSAX index; assumed to be `2^alphabet_num_bits-1` long;
     *        does not include `-inf` and `inf`
     */
    iSaxEnvelopeFinalizedIndex(const SeriesISaxProperties& series_isax_prop, vec<vec<iSaxWord>> first_isax_mins,
                               vec<vec<iSaxWord>> first_isax_maxs,
                               vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                               SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits, vec<float> breakpoints);

    ~iSaxEnvelopeFinalizedIndex() = default;

    vec<SearchResult> search(const vec<vec<float>>& query, const SearchOptions& search_options) const override;

   private:
    unsigned m_segment_len;
    vec<vec<vec<SaxSymbolT>>> m_first_sax_mins, m_first_sax_maxs;
    vec<std::unique_ptr<iSaxFinalizedNode>> m_first_layer_nodes;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    SaxSegIndT m_num_seg_per_channel;
    vec<float> m_breakpoints;

    std::pair<float, float> get_segment_limits(SaxNumBitsT num_bits, SaxSymbolT min_symbol,
                                               SaxSymbolT max_symbol) const;

    MAKE_SERIALIZABLE((m_segment_len, m_series_len, m_pos_per_env, m_first_sax_mins, m_first_sax_maxs,
                       m_first_layer_nodes, m_first_layer_num_bits, m_alphabet_num_bits, m_num_seg_per_channel,
                       m_num_channels, m_breakpoints));
};

#endif  // I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
