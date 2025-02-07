#ifndef ISAX_ULISSE_ENVELOPE_INDEX_HPP
#define ISAX_ULISSE_ENVELOPE_INDEX_HPP

#include <unordered_map>

#include "Search/EnvelopeIndex.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"

struct SaxSymbolsHash {
    std::size_t operator()(const vec<vec<SaxSymbolT>> &symbols) const;
};

class iSaxEnvelopeIndex : public IEnvelopeIndex {
   public:
    /**
     * @brief Construct a new iSaxEnvelopeIndex object
     *
     * @param series_isax_prop Properties of the time series
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param leaf_capacity Capacity of the leaf nodes
     * @param breakpoint_strategy Breakpoint strategy
     * @param split_strategy Split strategy
     * @param num_bits_limit Maximum number of bits used for any symbol in any node of the index
     */
    iSaxEnvelopeIndex(const SeriesISaxProperties &series_isax_prop, SaxNumBitsT first_layer_num_bits,
                      size_t leaf_capacity, std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                      std::unique_ptr<IiSaxSplitStrategy> split_strategy,
                      SaxNumBitsT num_bits_limit = DEFAULT_NUM_BIT_LIMIT);

    iSaxEnvelopeIndex() = default;

    ~iSaxEnvelopeIndex() = default;

    void insert(const EnvelopeEntry &entry) override;

    std::unique_ptr<IEnvelopeFinalizedIndex> finalize() override;

    const iSaxSplittableNode *get_first_layer_node(const vec<iSaxWord> &isax_mins) const;

   private:
    std::unordered_map<vec<vec<SaxSymbolT>>, std::unique_ptr<iSaxSplittableNode>, SaxSymbolsHash> m_first_layer;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits, m_num_bits_limit;
    uint m_segment_len, m_series_len, m_pos_per_env;
    SaxSegIndT m_num_seg_per_channel;
    MtsNumChannelsT m_num_channels;
    size_t m_leaf_capacity;
    std::unique_ptr<IiSaxBreakpointStrategy> m_breakpoint_strategy;
    vec<float> m_breakpoints;
    std::unique_ptr<IiSaxSplitStrategy> m_split_strategy;

    void split_leaf(vec<iSaxWord> &isax_min, std::unique_ptr<iSaxSplittableNode> &node_ref);
};

#endif  // ISAX_ULISSE_ENVELOPE_INDEX_HPP
