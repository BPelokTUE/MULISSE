#include "Summarization/iSaxWord.hpp"
#include "Summarization/SaxWord.hpp"
#include "Util/typedefs.hpp"

iSaxWord::iSaxWord(const vec<float> &paa, const iSaxWordSettings &settings)
    : SaxWord(paa, settings.alphabet_num_bits, settings.breakpoints), m_num_bits(settings.num_bits) {
    assert(paa.size() == settings.num_bits.size());
    assert(m_alphabet_num_bits >= *std::max_element(settings.num_bits.begin(), settings.num_bits.end()));
}

iSaxWord::iSaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT alphabet_num_bits)
    : SaxWord(symbols, alphabet_num_bits), m_num_bits(symbols.size(), alphabet_num_bits) {}

iSaxWord::iSaxWord(vec<SaxSymbolT> symbols, vec<SaxNumBitsT> num_bits, SaxNumBitsT alphabet_num_bits)
    : SaxWord(symbols, alphabet_num_bits), m_num_bits(num_bits) {
    assert(symbols.size() == num_bits.size());
    assert(alphabet_num_bits >= *std::max_element(num_bits.begin(), num_bits.end()));
}

SaxSymbolT iSaxWord::operator[](SaxSegIndT index) const {
    return m_symbols[index] >> (m_alphabet_num_bits - m_num_bits[index]);
}

SaxSymbolT iSaxWord::symbol_no_shift(SaxSegIndT index) const { return m_symbols[index]; }

const vec<SaxNumBitsT> &iSaxWord::get_num_bits() const { return m_num_bits; }

uint8_t iSaxWord::apply_split(SaxSegIndT seg_ind) {
    assert(m_num_bits[seg_ind] < m_alphabet_num_bits);

    ++m_num_bits[seg_ind];
    return operator[](seg_ind) & 1;
}

void iSaxWord::unsplit(SaxSegIndT seg_ind) {
    assert(m_num_bits[seg_ind] > 1);

    --m_num_bits[seg_ind];
}

void iSaxWord::set_new_bit(SaxSegIndT seg_ind, uint8_t bit) {
    SaxSymbolT mask = bit << (m_alphabet_num_bits - m_num_bits[seg_ind]);
    m_symbols[seg_ind] = (m_symbols[seg_ind] & ~mask) | mask;
}

void iSaxWord::append_to_symbol(SaxSegIndT index, uint8_t bit) {
    m_symbols[index] = (symbol_no_shift(index) << 1) | bit;
    m_num_bits[index]++;
    m_alphabet_num_bits = std::max(m_alphabet_num_bits, m_num_bits[index]);
}

void iSaxWord::set_symbol(SaxSegIndT index, SaxNumBitsT num_bits, SaxSymbolT symbol) {
    m_alphabet_num_bits = std::max(m_alphabet_num_bits, num_bits);
    m_symbols[index] = symbol;
    m_num_bits[index] = num_bits;
}

void iSaxWord::select_max_symbols(const iSaxWord &other) {
    assert(m_alphabet_num_bits == other.m_alphabet_num_bits);

    for (size_t i = 0; i < m_symbols.size(); ++i) {
        assert(m_num_bits[i] == other.m_num_bits[i]);
        m_symbols[i] = std::max(m_symbols[i], other.m_symbols[i]);
    }
}
