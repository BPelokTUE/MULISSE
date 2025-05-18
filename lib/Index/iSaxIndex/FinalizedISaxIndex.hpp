#ifndef ISAX_FINALIZED_INDEX_HPP
#define ISAX_FINALIZED_INDEX_HPP

#include <fstream>
#include <queue>

#include "Index/FinalizedIndex.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Index/Traits/SaxTraits.hpp"
#include "Index/iSaxIndex/FinalizedISaxNode.hpp"
#include "Serialization/Macros.hpp"
#include "Util/Constants/Math.hpp"

/**
 * @brief Priority queue entry, intended to be used in FinalizedISaxIndex
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
struct PQueueISaxEntry {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;

    Real m_min_dist_squared;
    vec<iSaxType> m_isax_words;
    const FinalizedISaxNode<FTag>* m_node;

    bool operator<(const PQueueISaxEntry& other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

/**
 * @brief Finalized iSAX index
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class FinalizedISaxIndex : public IFinalizedIndex<FTag> {
    using iSaxType = typename SaxTraits<FTag>::iSaxType;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

   public:
    FinalizedISaxIndex() = default;

    /**
     * @brief Construct a new FinalizedISaxIndex object
     *
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param first_layer_symbols Symbols of the first layer
     * @param first_layer_nodes First layer nodes
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param alphabet_num_bits The maximum number of bits used for any symbol in any node of the index
     * @param breakpoints Breakpoints used for the iSAX index; assumed to be `2^alphabet_num_bits-1` long;
     *        does not include `-inf` and `inf`
     * @param pos_per_env Number of positions per envelope. 0 if not applicable.
     */
    FinalizedISaxIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                       vec<vec<vec<SymbolType>>> first_layer_symbols,
                       vec<uptr<FinalizedISaxNode<FTag>>> first_layer_nodes, SaxNumBitsT first_layer_num_bits,
                       SaxNumBitsT alphabet_num_bits, vec<Real> breakpoints, uint pos_per_env = 0)
        : m_ch_segmentation_strategy(ch_segmentation_strategy),
          m_pos_per_env(pos_per_env),
          m_first_layer_symbols(std::move(first_layer_symbols)),
          m_first_layer_nodes(std::move(first_layer_nodes)),
          m_first_layer_num_bits(first_layer_num_bits),
          m_alphabet_num_bits(alphabet_num_bits),
          m_breakpoints(breakpoints) {  // TODO: try to remove this copy
        assert(m_first_layer_symbols.size() > 0);
    }

    ~FinalizedISaxIndex() = default;

    /**
     * @brief Get the lower and upper interval limits of the given symbol
     * @param num_bits Number of bits used for the symbol
     * @param symbol The symbol to get the limits for
     * @return A pair of lower and upper limits for the interval
     */
    std::pair<Real, Real> get_interval_limits(SaxNumBitsT num_bits, SymbolType symbol) const {
        uint num_shift = m_alphabet_num_bits - num_bits;
        auto [lower_ind, upper_ind] = get_limit_breakpoint_indexes(symbol, num_shift);
        return {
            lower_ind == -1 ? -INF : m_breakpoints[U(lower_ind)],
            upper_ind == m_breakpoints.size() ? INF : m_breakpoints[U(upper_ind)],
        };
    }

    /**
     * @brief Get the iSAX words of the left and right children after splitting on the given split index
     * @param node The node to get the child iSAX words from
     * @param isax_words The iSAX words of the node
     * @param c The channel index of the split
     * @param s The segment index of the split
     * @return A pair of iSAX words for the left and right children
     */
    std::pair<vec<iSaxType>, vec<iSaxType>> get_children_isax_words(const FinalizedISaxNode<FTag>* node,
                                                                    vec<iSaxType> isax_words, MtsNumChannelsT c,
                                                                    SaxSegIndT s) const;

    /**
     * @brief Get the symbols of the nodes in the first layer of the index
     * @return The symbols of the nodes in the first layer
     */
    const vec<vec<vec<SymbolType>>>& get_first_layer_symbols() const { return m_first_layer_symbols; }

    /**
     * @brief Get the number of bits used by the nodes in the first layer of the index
     * @return The number of bits used by the nodes in the first layer
     */
    SaxNumBitsT get_first_layer_num_bits() const { return m_first_layer_num_bits; }

    /**
     * @brief Get the first layer node of the index at the given index
     * @param ind The index of the node in teh first layer
     * @return The first layer node at the given index
     */
    const FinalizedISaxNode<FTag>* get_first_layer_node(size_t ind) const { return m_first_layer_nodes[ind].get(); }

    /**
     * @brief Get the channel segmentation strategy
     * @return The channel segmentation strategy
     */
    const IChannelSegmentationStrategy* get_ch_segmentation_strategy() const {
        return m_ch_segmentation_strategy.get();
    }

   private:
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    uint m_pos_per_env;
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
    vec<vec<vec<SymbolType>>> m_first_layer_symbols;
    vec<uptr<FinalizedISaxNode<FTag>>> m_first_layer_nodes;
    vec<Real> m_breakpoints;

    std::pair<int, int> get_limit_breakpoint_indexes(SymbolType symbol, uint num_shift) const;

    MAKE_SERIALIZABLE((m_first_layer_num_bits, m_alphabet_num_bits, m_pos_per_env, m_ch_segmentation_strategy,
                       m_first_layer_symbols, m_first_layer_nodes, m_breakpoints));
};

#endif  // ISAX_FINALIZED_INDEX_HPP
