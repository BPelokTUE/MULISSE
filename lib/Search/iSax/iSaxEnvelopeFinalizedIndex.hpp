#ifndef I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
#define I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include "Search/IEnvelopeIndex.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"

/** @brief Finalized iSAX (ULISSE) index */
class iSaxEnvelopeFinalizedIndex : public IFinalizedEnvelopeIndex {
   public:
    iSaxEnvelopeFinalizedIndex() = default;

    /**
     * @brief Construct a new iSaxEnvelopeFinalizedIndex object
     *
     * @param first_isax_mins iSAX min words of the first layer. Will be treated as SAX words.
     * @param first_isax_maxs iSAX max words of the first layer. Will be treated as SAX words.
     * @param first_layer_nodes First layer nodes
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param alphabet_num_bits The maximum number of bits used for any symbol in any node of the index
     * @param breakpoints Breakpoints used for the iSAX index; assumed to be `2^alphabet_num_bits-1` long;
     *        does not include `-inf` and `inf`
     */
    iSaxEnvelopeFinalizedIndex(vec<vec<iSaxWord>> first_isax_mins, vec<vec<iSaxWord>> first_isax_maxs,
                               vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                               SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits, vec<float> breakpoints);

    ~iSaxEnvelopeFinalizedIndex() = default;

    vec<FilePositionT> search(const vec<vec<float>>& query, const SearchOptions& search_options) const override;

   private:
    vec<vec<vec<SaxSymbolT>>> m_first_sax_mins, m_first_sax_maxs;
    vec<std::unique_ptr<iSaxFinalizedNode>> m_first_layer_nodes;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    SaxSegIndT m_num_seg_per_channel;
    MtsNumChannelsT m_num_channels;
    vec<float> m_breakpoints;

    MAKE_SERIALIZABLE((m_first_sax_mins, m_first_sax_maxs, m_first_layer_nodes, m_first_layer_num_bits,
                       m_alphabet_num_bits, m_num_seg_per_channel, m_num_channels, m_breakpoints));
};

#endif  // I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
