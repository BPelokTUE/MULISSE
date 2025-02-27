#include "Search/iSax/iSaxIndex.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

std::size_t SaxSymbolsHash::operator()(const vec<vec<SaxSymbolT>> &symbols) const {
    std::size_t seed = 0, num_symbols = symbols[0].size();
    for (auto channel : symbols) {
        for (SaxSymbolT symbol : channel) {
            boost::hash_combine(seed, symbol);
        }
    }
    return seed;
}

using FTagEnv = typename iSaxIndexTraits<Envelope>::FinalizedTag;
using SymbolTypeEnv = typename SaxTraits<FTagEnv>::SymbolType;

std::pair<uptr<iSaxFinalizedNode<FTagEnv>>, vec<vec<SymbolTypeEnv>>> iSaxEnvelopeIndex::finalize_first_layer_node(
    vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<Envelope>> &node, iSaxWordSettings &isax_word_settings) {
    auto [finalized_node, isax_max] = get_envelope_node_finalization_result(node, isax_word_settings);

    MtsNumChannelsT num_channels = m_series_isax_prop->num_channels;
    SaxSegIndT num_seg_per_channel = m_series_isax_prop->num_seg_per_channel;

    vec<vec<SymbolTypeEnv>> symbols(num_channels, vec<SymbolTypeEnv>(num_seg_per_channel));
    SaxNumBitsT shift = m_alphabet_num_bits - m_first_layer_num_bits;
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s)
            symbols[c][s] = SymbolTypeEnv(key_symbols[c][s], isax_max[c].symbol_no_shift(s) >> shift);
    }
    return {std::move(finalized_node), std::move(symbols)};
}

iSaxEnvelopeIndex::iSaxEnvelopeIndex(const SeriesISaxProperties &series_isax_prop, SaxNumBitsT first_layer_num_bits,
                                     size_t leaf_capacity, uptr<IiSaxSplitStrategy<Envelope>> split_strategy)
    : iSaxIndex(series_isax_prop, first_layer_num_bits, leaf_capacity, std::move(split_strategy)) {}
