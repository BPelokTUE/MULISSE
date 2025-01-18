#include "Summarization/SaxWord.hpp"

SaxWord::SaxWord(const vec<float> &paa, SaxNumBitsT num_bits, const vec<float> &breakpoints) {
    m_max_num_bits = num_bits;
    unsigned paa_len = paa.size();
    m_symbols.resize(paa_len);

    for (unsigned i = 0; i < paa_len; ++i) {
        auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), paa[i]);
        m_symbols[i] = std::distance(breakpoints.begin(), it);
    }
}

const unsigned &SaxWord::operator[](std::size_t index) const { return m_symbols[index]; }

void SaxWord::set_symbol(SaxSplitIndT index, SaxSymbolT symbol) { m_symbols[index] = symbol; }
