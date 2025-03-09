#include "Util/typedefs.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/iSaxWord.hpp"

// PaaISax

PaaISax::PaaISax(vec<PaaSaxSymbol> paa_sax_symbol, SaxNumBitsT num_bits) {
    vec<SaxSymbolT> symbols(paa_sax_symbol.size());
    for (size_t i = 0; i < symbols.size(); ++i) {
        symbols[i] = paa_sax_symbol[i].symbol;
    }
    isax_word = iSaxWord(symbols, num_bits);
};

const vec<SaxNumBitsT> &PaaISax::get_num_bits() const { return isax_word.get_num_bits(); }

PaaSaxSymbol PaaISax::symbol_no_shift(SaxSegIndT index) const { return {isax_word.symbol_no_shift(index)}; }

size_t PaaISax::size() const { return isax_word.size(); }

template <>
pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedInternal<PaaTag>::get_children_max_symbols(
    SaxNumBitsT split_num_bits, SaxNumBitsT symbol_num_bits) const {
    return iSaxFinalizedNode<PaaTag>::get_children_max_symbols(split_num_bits, symbol_num_bits);
}

template <>
pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedLeaf<PaaTag>::get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                                 SaxNumBitsT symbol_num_bits) const {
    return iSaxFinalizedNode<PaaTag>::get_children_max_symbols(split_num_bits, symbol_num_bits);
}

// EnvelopeISax

EnvelopeISax::EnvelopeISax(vec<EnvelopeSaxSymbol> envelope_sax_symbol, SaxNumBitsT num_bits) {
    vec<SaxSymbolT> min_symbols(envelope_sax_symbol.size()), max_symbols(envelope_sax_symbol.size());

    for (size_t i = 0; i < min_symbols.size(); ++i) {
        min_symbols[i] = envelope_sax_symbol[i].min_symbol;
        max_symbols[i] = envelope_sax_symbol[i].max_symbol;
    }
    isax_min = iSaxWord(min_symbols, num_bits);
    isax_max = iSaxWord(max_symbols, num_bits);
};

const vec<SaxNumBitsT> &EnvelopeISax::get_num_bits() const { return isax_min.get_num_bits(); }

EnvelopeSaxSymbol EnvelopeISax::symbol_no_shift(SaxSegIndT index) const {
    return {isax_min.symbol_no_shift(index), isax_max.symbol_no_shift(index)};
}

size_t EnvelopeISax::size() const { return isax_min.size(); }

template <>
pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedInternal<EnvelopeTag>::get_children_max_symbols(
    SaxNumBitsT split_num_bits, SaxNumBitsT symbol_num_bits) const {
    SaxNumBitsT shift = symbol_num_bits - split_num_bits - 1;
    auto envelope_args = static_cast<iSaxEnvelopeInternalNodeArgs *>(m_args.get());
    return {envelope_args->max_symbol_left >> shift, envelope_args->max_symbol_right >> shift};
}

template <>
pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedLeaf<EnvelopeTag>::get_children_max_symbols(
    SaxNumBitsT split_num_bits, SaxNumBitsT symbol_num_bits) const {
    return {-1, -1};
}
