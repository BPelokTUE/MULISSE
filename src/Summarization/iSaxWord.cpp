#include "Summarization/iSaxWord.hpp"

iSaxWord::iSaxWord(const vec<float> &paa, vec<SaxNumBitsT> num_bits, SaxNumBitsT alphabet_num_bits,
                   const vec<float> &breakpoints)
    : SaxWord(paa, alphabet_num_bits, breakpoints), m_num_bits(num_bits) {}

iSaxWord::iSaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT alphabet_num_bits)
    : SaxWord(symbols, alphabet_num_bits), m_num_bits(symbols.size(), alphabet_num_bits) {}

iSaxWord::iSaxWord(vec<SaxSymbolT> symbols, vec<SaxNumBitsT> num_bits, SaxNumBitsT alphabet_num_bits)
    : SaxWord(symbols, alphabet_num_bits), m_num_bits(num_bits) {}

SaxSymbolT iSaxWord::operator[](std::size_t index) const {
    auto val = m_symbols[index] >> (m_alphabet_num_bits - m_num_bits[index]);
    return val;
}

const vec<SaxNumBitsT> &iSaxWord::get_num_bits() const { return m_num_bits; }

uint8_t iSaxWord::apply_split(SaxSegIndT split_ind) {
    assert(m_num_bits[split_ind] < m_alphabet_num_bits);

    ++m_num_bits[split_ind];
    return operator[](split_ind) & 1;
}

void iSaxWord::append_to_symbol(SaxSegIndT index, uint8_t bit) {
    m_symbols[index] = (m_symbols[index] << 1) | bit;
    m_num_bits[index]++;
    m_alphabet_num_bits = std::max(m_alphabet_num_bits, m_num_bits[index]);
}

void iSaxWord::remove_from_symbol(SaxSegIndT index) {
    m_symbols[index] >>= 1;
    m_num_bits[index]--;
}
