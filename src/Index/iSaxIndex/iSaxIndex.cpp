#include "Index/iSaxIndex/iSaxIndex.hpp"

#include "Index/EntryInserter/EntryInserter.hpp"
#include "Index/EntryInserter/ParallelInserter.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/Traits/FinalizedTraits.hpp"
#include "Index/Traits/IndexTraits.hpp"
#include "Index/iSaxIndex/FinalizedISaxNode.hpp"

// iSaxIndex

template <typename T>
    requires DerivedFromEntryData<T>
void iSaxIndex<T>::insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) {
    uptr<IEntryInserter<iSaxIndex<T>>> inserter;
    switch (inserter_type) {
        case TOP_DOWN:
            inserter = std::make_unique<TopDownInserter<iSaxIndex<T>>>(this->shared_from_this());
            break;
        case PARALLEL:
            inserter = std::make_unique<iSaxParallelInserter<T>>(this->shared_from_this());
            break;
        default:
            throw std::invalid_argument("Invalid inserter type");
    }
    inserter->insert_entries(entries);
}

// iSaxPaaIndex

using FTagPaa = typename IndexTraits<Paa>::FinalizedTag;
using SymbolTypePaa = typename SaxTraits<FTagPaa>::SymbolType;

std::pair<uptr<FinalizedISaxNode<FTagPaa>>, vec<vec<SymbolTypePaa>>> iSaxPaaIndex::finalize_first_layer_node(
    vec<vec<SaxSymbolT>> key_symbols, uptr<SplittableISaxNode<Paa>> &node) {
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

std::pair<uptr<FinalizedISaxNode<FTagEnv>>, vec<vec<SymbolTypeEnv>>> iSaxEnvelopeIndex::finalize_first_layer_node(
    vec<vec<SaxSymbolT>> key_symbols, uptr<SplittableISaxNode<Envelope>> &node) {
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
