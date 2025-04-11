#include <cassert>
#include <algorithm>

#include "Summarization/SaxWord.hpp"
#include "Util/typedefs.hpp"

SaxWord::SaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT num_bits) : m_symbols(symbols), m_alphabet_num_bits(num_bits) {
    assert(num_bits > 0);
};

SaxSymbolT SaxWord::operator[](SaxSegIndT index) const { return m_symbols[index]; }

bool SaxWord::operator==(const SaxWord &other) const {
    assert(size() == other.size());

    SaxSegIndT num_symbols = static_cast<SaxSegIndT>(m_symbols.size());
    for (SaxSegIndT i = 0; i < num_symbols; ++i) {
        if (operator[](i) != other[i]) return false;
    }
    return true;
}

size_t SaxWord::size() const { return m_symbols.size(); }

SaxNumBitsT SaxWord::get_alphabet_num_bits() const { return m_alphabet_num_bits; }
