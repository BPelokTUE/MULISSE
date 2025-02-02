#include <fstream>

#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"

iSaxEnvelopeFinalizedIndex::iSaxEnvelopeFinalizedIndex(vec<vec<iSaxWord>> first_isax_mins,
                                                       vec<vec<iSaxWord>> first_isax_maxs,
                                                       vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                                                       SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits,
                                                       vec<float> breakpoints)
    : m_first_layer_nodes(std::move(first_layer_nodes)),
      m_first_layer_num_bits(first_layer_num_bits),
      m_alphabet_num_bits(alphabet_num_bits),
      m_num_seg_per_channel(first_isax_mins[0][0].size()),
      m_num_channels(first_isax_mins[0].size()),
      m_breakpoints(std::move(breakpoints)) {
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

vec<FilePositionT> iSaxEnvelopeFinalizedIndex::search(const vec<vec<float>>& query,
                                                      const SearchOptions& search_options) const {
    return {};
};
