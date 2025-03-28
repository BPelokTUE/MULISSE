#include "Summarization/iSaxWord.hpp"
#include "Summarization/SaxWord.hpp"
#include "Util/typedefs.hpp"

iSaxWord::iSaxWord(const vec<Real> &paa, const iSaxWordSettings &settings)
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
    SaxSymbolT mask = 1 << (m_alphabet_num_bits - m_num_bits[seg_ind]);
    SaxSymbolT shifted_bit = bit << (m_alphabet_num_bits - m_num_bits[seg_ind]);
    m_symbols[seg_ind] = (m_symbols[seg_ind] & ~mask) | shifted_bit;
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

std::optional<Real> iSaxWord::get_mid_breakpoint(SaxSegIndT segment_ind, const vec<Real> &breakpoints) const {
    uint alphabet_ratio = (breakpoints.size() + 1) / (1 << (m_num_bits[segment_ind] + 1));
    // If this segment already has the maximum allowed cardinality ==> cannot be split further
    if (alphabet_ratio == 0) return std::nullopt;

    auto symbol = operator[](segment_ind);
    // `symbol * 2 + 1` goes to the upper interval in the next resolution
    // `* (alphabet_size_ratio >> 1)` goes to the lowest portion of the upper interval
    // (i.e. just above the mid breakpoint) in the desired resolution
    // `-1` adjusts for the fact that the breakpoints have an implicit -inf at the beginning
    return breakpoints.at((symbol * 2 + 1) * (alphabet_ratio >> 1) - 1);
}
