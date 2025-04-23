#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"

// iSaxFinalizedIndex<Paa>

template <>
std::pair<int, int> iSaxFinalizedIndex<PaaTag>::get_limit_breakpoint_indexes(PaaSaxSymbol symbol,
                                                                             uint num_shift) const {
    return {(symbol.m_symbol << num_shift) - 1, ((symbol.m_symbol + 1) << num_shift) - 1};
}

template <>
std::pair<vec<PaaISax>, vec<PaaISax>> iSaxFinalizedIndex<PaaTag>::get_children_isax_words(
    const iSaxFinalizedNode<PaaTag> *node, vec<PaaISax> isax_words, MtsNumChannelsT c, SaxSegIndT s) const {
    vec<PaaISax> left_isax_words = isax_words;
    left_isax_words[c].m_isax_word.append_to_symbol(s, 0);
    vec<PaaISax> right_isax_words = isax_words;
    right_isax_words[c].m_isax_word.append_to_symbol(s, 1);

    return {left_isax_words, right_isax_words};
}

// iSaxFinalizedIndex<EnvelopeTag>

template <>
std::pair<int, int> iSaxFinalizedIndex<EnvelopeTag>::get_limit_breakpoint_indexes(EnvelopeSaxSymbol symbol,
                                                                                  uint num_shift) const {
    return {(symbol.m_min_symbol << num_shift) - 1, ((symbol.m_max_symbol + 1) << num_shift) - 1};
}

template <>
std::pair<vec<EnvelopeISax>, vec<EnvelopeISax>> iSaxFinalizedIndex<EnvelopeTag>::get_children_isax_words(
    const iSaxFinalizedNode<EnvelopeTag> *node, vec<EnvelopeISax> isax_words, MtsNumChannelsT c, SaxSegIndT s) const {
    SaxNumBitsT num_bits = isax_words[c].get_num_bits()[s];
    auto [max_symbol_left, max_symbol_right] =
        static_cast<const iSaxFinalizedNode<EnvelopeTag> *>(node)->get_children_max_symbols(num_bits,
                                                                                            m_alphabet_num_bits);
    vec<EnvelopeISax> left_isax_words = isax_words;
    left_isax_words[c].m_isax_min.append_to_symbol(s, 0);
    left_isax_words[c].m_isax_max.set_symbol(s, num_bits + 1, max_symbol_left);
    vec<EnvelopeISax> right_isax_words = std::move(isax_words);
    right_isax_words[c].m_isax_min.append_to_symbol(s, 1);
    right_isax_words[c].m_isax_max.set_symbol(s, num_bits + 1, max_symbol_right);

    return {std::move(left_isax_words), std::move(right_isax_words)};
}
