#ifndef I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
#define I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include "Util/typedefs.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"

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
};

/** @brief Properties of time series for iSAX envelope indexes */
struct SeriesISaxEnvelopeProperties : SeriesISaxProperties {
    /** @brief Size of starting position groups */
    uint pos_per_env;
};

struct EntryISax {
    virtual ~EntryISax() = default;
};

struct PaaISax : EntryISax {
    vec<iSaxWord> isax_words;
};

struct EnvelopeISax : EntryISax {
    vec<iSaxWord> isax_mins;
    vec<iSaxWord> isax_maxs;
};

class iSaxEnvelopeFinalizedIndex : public IFinalizedIndex<Envelope> {
   public:
    iSaxEnvelopeFinalizedIndex() = default;

    /**
     * @brief Construct a new iSaxEnvelopeFinalizedIndex object
     *
     * @param series_isax_prop Properties of the time series
     * @param first_layer_min_symbols Min SAX symbols of the first layer
     * @param first_layer_max_symbols Max SAX symbols of the first layer
     * @param first_layer_nodes First layer nodes
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param alphabet_num_bits The maximum number of bits used for any symbol in any node of the index
     * @param breakpoints Breakpoints used for the iSAX index; assumed to be `2^alphabet_num_bits-1` long;
     *        does not include `-inf` and `inf`
     */
    iSaxEnvelopeFinalizedIndex(uptr<SeriesISaxProperties> series_isax_prop,
                               vec<vec<vec<SaxSymbolT>>> first_layer_min_symbols,
                               vec<vec<vec<SaxSymbolT>>> first_layer_max_symbols,
                               vec<uptr<iSaxFinalizedNode>> first_layer_nodes, SaxNumBitsT first_layer_num_bits,
                               SaxNumBitsT alphabet_num_bits, vec<float> breakpoints);

    ~iSaxEnvelopeFinalizedIndex() = default;

    vec<SearchResult> search(const vec<vec<float>>& query, const SearchOptions& search_options,
                             std::ifstream& dataset_ifs) const override;

    const vec<vec<vec<SaxSymbolT>>>& get_first_layer_min_symbols() const;

    const vec<vec<vec<SaxSymbolT>>>& get_first_layer_max_symbols() const;

    SaxNumBitsT get_first_layer_num_bits() const;

   private:
    vec<vec<vec<SaxSymbolT>>> m_first_layer_min_symbols, m_first_layer_max_symbols;
    vec<std::unique_ptr<iSaxFinalizedNode>> m_first_layer_nodes;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    vec<float> m_breakpoints;
    uptr<SeriesISaxProperties> m_series_isax_prop;

    std::pair<float, float> get_segment_limits(SaxNumBitsT num_bits, SaxSymbolT min_symbol,
                                               SaxSymbolT max_symbol) const;

    MAKE_SERIALIZABLE((m_series_isax_prop, m_first_layer_min_symbols, m_first_layer_max_symbols, m_first_layer_nodes,
                       m_first_layer_num_bits, m_alphabet_num_bits, m_breakpoints));
};

#endif  // I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
