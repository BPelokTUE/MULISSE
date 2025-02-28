#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"

SeriesISaxProperties::SeriesISaxProperties(uint segment_len, uint series_len, MtsNumChannelsT num_channels,
                                           SaxSegIndT num_seg_per_channel)
    : segment_len(segment_len),
      series_len(series_len),
      num_channels(num_channels),
      num_seg_per_channel(num_seg_per_channel) {}

SeriesISaxEnvelopeProperties::SeriesISaxEnvelopeProperties(uint segment_len, uint series_len,
                                                           MtsNumChannelsT num_channels, SaxSegIndT num_seg_per_channel,
                                                           uint pos_per_env)
    : SeriesISaxProperties(segment_len, series_len, num_channels, num_seg_per_channel), pos_per_env(pos_per_env) {}

template <>
std::pair<float, float> iSaxFinalizedIndex<EnvelopeTag>::get_segment_limits(SaxNumBitsT num_bits,
                                                                            EnvelopeSaxSymbol symbol) const {
    uint num_shift = m_alphabet_num_bits - num_bits;
    int lower_ind = (symbol.min_symbol << num_shift) - 1, upper_ind = ((symbol.max_symbol + 1) << num_shift) - 1;
    return {
        lower_ind == -1 ? -INF : m_breakpoints[lower_ind],
        upper_ind == m_breakpoints.size() ? INF : m_breakpoints[upper_ind],
    };
}

template <>
std::pair<vec<EnvelopeISax>, vec<EnvelopeISax>> iSaxFinalizedIndex<EnvelopeTag>::get_children_isax_words(
    const iSaxFinalizedNode<EnvelopeTag> *node, vec<EnvelopeISax> isax_words, MtsNumChannelsT c, SaxSegIndT s) const {
    SaxNumBitsT num_bits = isax_words[c].get_num_bits()[s];
    auto [max_symbol_left, max_symbol_right] =
        static_cast<const iSaxFinalizedNode<EnvelopeTag> *>(node)->get_children_max_symbols(num_bits,
                                                                                            m_alphabet_num_bits);
    vec<EnvelopeISax> left_isax_words = isax_words;
    left_isax_words[c].isax_min.append_to_symbol(s, 0);
    left_isax_words[c].isax_max.set_symbol(s, num_bits, max_symbol_left);
    vec<EnvelopeISax> right_isax_words = std::move(isax_words);
    right_isax_words[c].isax_min.append_to_symbol(s, 1);
    right_isax_words[c].isax_max.set_symbol(s, num_bits, max_symbol_right);

    return {left_isax_words, right_isax_words};
}
