#include "Summarization/iSaxWord.hpp"

iSaxWord::iSaxWord(const vec<float> &paa, SaxNumBitsT start_num_bits, const vec<float> &breakpoints)
    : SaxWord(paa, start_num_bits, breakpoints), m_num_bits(paa.size(), start_num_bits) {}

iSaxWord::iSaxWord(vec<SaxSymbolT> symbols, vec<SaxNumBitsT> num_bits, SaxNumBitsT max_num_bits)
    : SaxWord(symbols, max_num_bits), m_num_bits(num_bits) {}

iSaxWord::iSaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT max_num_bits)
    : SaxWord(symbols, max_num_bits), m_num_bits(symbols.size(), max_num_bits) {}

const SaxNumBitsT &iSaxWord::get_num_bits(SaxSplitIndT index) const { return m_num_bits[index]; }

void iSaxWord::set_symbol_and_bits(SaxSplitIndT index, SaxSymbolT symbol, SaxNumBitsT bits) {
    m_symbols[index] = symbol;
    m_num_bits[index] = bits;
    m_max_num_bits = std::max(m_max_num_bits, bits);
}

uint8_t iSaxWord::apply_split(const vec<float> &paa, const vec<float> &breakpoints, SaxSplitIndT split_ind) {
    auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), paa[split_ind]);
    SaxSymbolT prev_symbol = m_symbols[split_ind];
    m_symbols[split_ind] = it - breakpoints.begin();
    return m_symbols[split_ind] - (prev_symbol << 1);
}
