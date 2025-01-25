#include <fstream>

#include "Search/iSax/iSaxFinalizedUliEnvIndex.hpp"

iSaxFinalizedUliEnvIndex::iSaxFinalizedUliEnvIndex(vec<vec<iSaxWord>> first_isax_mins,
                                                   vec<vec<iSaxWord>> first_isax_maxs,
                                                   vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                                                   SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits,
                                                   vec<float> breakpoints)
    : m_first_isax_mins(std::move(first_isax_mins)),
      m_first_isax_maxs(std::move(first_isax_maxs)),
      m_first_layer_nodes(std::move(first_layer_nodes)),
      m_first_layer_num_bits(first_layer_num_bits),
      m_alphabet_num_bits(alphabet_num_bits),
      m_num_seg_per_channel(m_first_isax_mins[0][0].size()),
      m_num_channels(m_first_isax_mins[0].size()) {}

void iSaxFinalizedUliEnvIndex::serialize(std::ofstream ofs) {}

void iSaxFinalizedUliEnvIndex::deserialize(std::ifstream ifs) {}
