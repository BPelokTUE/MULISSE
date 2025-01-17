#include "Summarization/iSaxWord.hpp"

iSaxWord::iSaxWord(const vec<float>& paa, iSaxNumBitsT start_num_bits, vec<float> breakpoints) {
    m_max_num_bits = start_num_bits;
    unsigned paa_len = paa.size();
    m_symbols.resize(paa_len);
    m_num_bits.resize(paa_len, m_max_num_bits);

    for (unsigned i = 0; i < paa_len; ++i) {
        auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), paa[i]);
        m_symbols[i] = std::distance(breakpoints.begin(), it);
    }
}

const unsigned& iSaxWord::operator[](std::size_t index) const { return m_symbols[index]; }

void iSaxWord::set_symbol(iSaxSplitIndT index, iSaxSymbolT symbol) { m_symbols[index] = symbol; }

void iSaxWord::set_symbol(iSaxSplitIndT index, iSaxSymbolT symbol, iSaxNumBitsT bits) {
    m_symbols[index] = symbol;
    m_num_bits[index] = bits;
}

std::pair<iSaxWord, iSaxWord> iSaxWord::split(iSaxSplitIndT index) const {
    iSaxWord left = *this;

    iSaxNumBitsT new_num_bits = left.m_num_bits[index] + 1;
    iSaxNumBitsT new_max_bits = std::max(new_num_bits, m_max_num_bits);

    left.set_symbol(index, m_symbols[index] << 1, new_max_bits);
    iSaxWord right = *this;
    right.set_symbol(index, m_symbols[index] + 1);

    return std::make_pair(left, right);
}
