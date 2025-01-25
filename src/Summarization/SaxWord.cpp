#include <cassert>
#include <algorithm>

#include "Summarization/SaxWord.hpp"

SaxWord::SaxWord(const vec<float> &paa, SaxNumBitsT num_bits, const vec<float> &breakpoints) {
    assert(num_bits > 0);
    assert(breakpoints.size() == (1 << num_bits) - 1);
    assert(std::is_sorted(breakpoints.begin(), breakpoints.end()));

    m_alphabet_num_bits = num_bits;
    unsigned paa_len = paa.size();
    m_symbols.resize(paa_len);

    for (unsigned i = 0; i < paa_len; ++i) {
        auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), paa[i]);
        m_symbols[i] = it - breakpoints.begin();
    }
}

SaxWord::SaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT num_bits) : m_symbols(symbols), m_alphabet_num_bits(num_bits) {
    assert(num_bits > 0);
};

SaxSymbolT SaxWord::operator[](std::size_t index) const { return m_symbols[index]; }

bool SaxWord::operator==(const SaxWord &other) const {
    assert(size() == other.size());

    for (size_t i = 0; i < m_symbols.size(); ++i) {
        if (operator[](i) != other[i]) return false;
    }
    return true;
}

size_t SaxWord::size() const { return m_symbols.size(); }

SaxNumBitsT SaxWord::get_alphabet_num_bits() const { return m_alphabet_num_bits; }
