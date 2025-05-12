#include "Index/iSaxIndex/FinalizedISaxNode.hpp"

// PaaISax

PaaISax::PaaISax(vec<PaaSaxSymbol> paa_sax_symbol, SaxNumBitsT num_bits) {
    vec<SaxSymbolT> symbols(paa_sax_symbol.size());
    for (size_t i = 0; i < symbols.size(); ++i) {
        symbols[i] = paa_sax_symbol[i].m_symbol;
    }
    m_isax_word = iSaxWord(symbols, num_bits);
};

const vec<SaxNumBitsT> &PaaISax::get_num_bits() const { return m_isax_word.get_num_bits(); }

PaaSaxSymbol PaaISax::symbol_no_shift(SaxSegIndT index) const { return {m_isax_word.symbol_no_shift(index)}; }

size_t PaaISax::size() const { return m_isax_word.size(); }

template <>
pair<SaxSymbolT, SaxSymbolT> FinalizedISaxInternal<PaaTag>::get_children_max_symbols(
    SaxNumBitsT split_num_bits, SaxNumBitsT symbol_num_bits) const {
    return FinalizedISaxNode<PaaTag>::get_children_max_symbols(split_num_bits, symbol_num_bits);
}

template <>
pair<SaxSymbolT, SaxSymbolT> FinalizedISaxLeaf<PaaTag>::get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                                 SaxNumBitsT symbol_num_bits) const {
    return FinalizedISaxNode<PaaTag>::get_children_max_symbols(split_num_bits, symbol_num_bits);
}

// EnvelopeISax

EnvelopeISax::EnvelopeISax(vec<EnvelopeSaxSymbol> envelope_sax_symbol, SaxNumBitsT num_bits) {
    vec<SaxSymbolT> min_symbols(envelope_sax_symbol.size()), max_symbols(envelope_sax_symbol.size());

    for (size_t i = 0; i < min_symbols.size(); ++i) {
        min_symbols[i] = envelope_sax_symbol[i].m_min_symbol;
        max_symbols[i] = envelope_sax_symbol[i].m_max_symbol;
    }
    m_isax_min = iSaxWord(min_symbols, num_bits);
    m_isax_max = iSaxWord(max_symbols, num_bits);
};

const vec<SaxNumBitsT> &EnvelopeISax::get_num_bits() const { return m_isax_min.get_num_bits(); }

EnvelopeSaxSymbol EnvelopeISax::symbol_no_shift(SaxSegIndT index) const {
    return {m_isax_min.symbol_no_shift(index), m_isax_max.symbol_no_shift(index)};
}

size_t EnvelopeISax::size() const { return m_isax_min.size(); }

template <>
pair<SaxSymbolT, SaxSymbolT> FinalizedISaxInternal<EnvelopeTag>::get_children_max_symbols(
    SaxNumBitsT split_num_bits, SaxNumBitsT symbol_num_bits) const {
    SaxNumBitsT shift = static_cast<SaxNumBitsT>(symbol_num_bits - split_num_bits - 1);
    auto envelope_args = static_cast<iSaxEnvelopeInternalNodeArgs *>(m_args.get());
    return {envelope_args->m_max_symbol_left >> shift, envelope_args->m_max_symbol_right >> shift};
}

template <>
pair<SaxSymbolT, SaxSymbolT> FinalizedISaxLeaf<EnvelopeTag>::get_children_max_symbols(
    SaxNumBitsT split_num_bits, SaxNumBitsT symbol_num_bits) const {
    return {-1, -1};
}
