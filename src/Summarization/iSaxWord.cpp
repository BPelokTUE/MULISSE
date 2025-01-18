#include "Summarization/iSaxWord.hpp"

iSaxWord::iSaxWord(const vec<float> &paa, SaxNumBitsT start_num_bits, const vec<float> &breakpoints)
    : SaxWord(paa, start_num_bits, breakpoints), m_num_bits(paa.size(), start_num_bits) {}

const SaxNumBitsT &iSaxWord::get_num_bits(SaxSplitIndT index) const { return m_num_bits[index]; }

void iSaxWord::set_symbol_and_bits(SaxSplitIndT index, SaxSymbolT symbol, SaxNumBitsT bits) {
    m_symbols[index] = symbol;
    m_num_bits[index] = bits;
    m_max_num_bits = std::max(m_max_num_bits, bits);
}

std::pair<iSaxWord, iSaxWord> iSaxWord::split(SaxSplitIndT index) const {
    iSaxWord left(*this);
    left.set_symbol_and_bits(index, m_symbols[index] << 1, left.m_num_bits[index] + 1);

    iSaxWord right(left);
    right.set_symbol(index, m_symbols[index] + 1);

    return std::make_pair(left, right);
}
