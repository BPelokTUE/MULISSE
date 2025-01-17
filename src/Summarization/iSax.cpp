#include "Summarization/iSax.hpp"

iSaxWord::iSaxWord(const vec<float>& paa, unsigned start_num_bits, vec<float> breakpoints) {
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

void iSaxWord::set_symbol(unsigned index, unsigned symbol) { m_symbols[index] = symbol; }

void iSaxWord::set_symbol(unsigned index, unsigned symbol, unsigned bits) {
    m_symbols[index] = symbol;
    m_num_bits[index] = bits;
}

std::pair<iSaxWord, iSaxWord> iSaxWord::split(unsigned index) const {
    iSaxWord left = *this;
    left.set_symbol(index, m_symbols[index] << 1, left.m_num_bits[index] + 1);
    iSaxWord right = *this;
    right.set_symbol(index, m_symbols[index] + 1);

    return std::make_pair(left, right);
}
