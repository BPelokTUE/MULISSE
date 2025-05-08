#include "Search/iSax/iSaxIndex.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/RunSettings.hpp"

// iSaxPaaIndex

using FTagPaa = typename IndexTraits<Paa>::FinalizedTag;
using SymbolTypePaa = typename SaxTraits<FTagPaa>::SymbolType;

std::pair<uptr<iSaxFinalizedNode<FTagPaa>>, vec<vec<SymbolTypePaa>>> iSaxPaaIndex::finalize_first_layer_node(
    vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<Paa>> &node) {
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(key_symbols.size());
    SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(key_symbols[0].size());

    vec<vec<SymbolTypePaa>> symbols(num_channels, vec<SymbolTypePaa>(num_seg_per_channel));
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) symbols[c][s] = SymbolTypePaa(key_symbols[c][s]);
    }
    auto finalized_node = get_paa_node_finalization_result(node);
    return {std::move(finalized_node), std::move(symbols)};
}

iSaxPaaIndex::iSaxPaaIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                           sptr<ISegmentationStrategy> segmentation_strategy,
                           uptr<IiSaxSplitStrategy<Paa>> split_strategy)
    : iSaxIndex(first_layer_num_bits, leaf_capacity, segmentation_strategy, std::move(split_strategy)) {}

// iSaxEnvelopeIndex

using FTagEnv = typename IndexTraits<Envelope>::FinalizedTag;
using SymbolTypeEnv = typename SaxTraits<FTagEnv>::SymbolType;

std::pair<uptr<iSaxFinalizedNode<FTagEnv>>, vec<vec<SymbolTypeEnv>>> iSaxEnvelopeIndex::finalize_first_layer_node(
    vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<Envelope>> &node) {
    auto [finalized_node, isax_max] = get_envelope_node_finalization_result(node);

    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(key_symbols.size());
    SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(key_symbols[0].size());

    vec<vec<SymbolTypeEnv>> symbols(num_channels, vec<SymbolTypeEnv>(num_seg_per_channel));
    SaxNumBitsT shift = m_alphabet_num_bits - m_first_layer_num_bits;
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s)
            symbols[c][s] = SymbolTypeEnv(key_symbols[c][s], isax_max[c].symbol_no_shift(s) >> shift);
    }
    return {std::move(finalized_node), std::move(symbols)};
}

iSaxEnvelopeIndex::iSaxEnvelopeIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                                     sptr<ISegmentationStrategy> segmentation_strategy,
                                     uptr<IiSaxSplitStrategy<Envelope>> split_strategy, uint pos_per_env)
    : iSaxIndex(first_layer_num_bits, leaf_capacity, segmentation_strategy, std::move(split_strategy)) {}
