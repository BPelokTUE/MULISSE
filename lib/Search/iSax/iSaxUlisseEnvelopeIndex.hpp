#ifndef ISAX_ULISSE_ENVELOPE_INDEX_HPP
#define ISAX_ULISSE_ENVELOPE_INDEX_HPP

#include <unordered_map>

#include "Search/IUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"

struct iSaxWordVecHash {
    std::size_t operator()(const vec<iSaxWord> &isax_mins) const;
};

class iSaxUlisseEnvelopeIndex : IUlisseEnvelopeIndex {
   public:
    iSaxUlisseEnvelopeIndex(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels,
                            SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                            std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                            std::unique_ptr<IiSaxSplitStrategy> split_strategy,
                            SaxNumBitsT num_bits_limit = DEFAULT_NUM_BIT_LIMIT);

    iSaxUlisseEnvelopeIndex() = default;

    ~iSaxUlisseEnvelopeIndex() = default;

    void insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) override;

    const iSaxNode *get_first_layer_node(const vec<iSaxWord> &isax_mins) const;

    vec<FilePositionT> search(vec<vec<float>> mts, const SearchOptions &search_options) const override;

   private:
    std::unordered_map<vec<iSaxWord>, std::unique_ptr<iSaxNode>, iSaxWordVecHash> m_first_layer;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits, m_num_bits_limit;
    SaxSegIndT m_num_seg_per_channel;
    MtsNumChannelsT m_num_channels;
    size_t m_leaf_capacity;
    std::unique_ptr<IiSaxBreakpointStrategy> m_breakpoint_strategy;
    vec<float> m_breakpoints;
    std::unique_ptr<IiSaxSplitStrategy> m_split_strategy;

    void split_leaf(vec<iSaxWord> &isax_min, const vec<UlisseEnvelope> &envelopes, std::unique_ptr<iSaxNode> &node_ref);
};

#endif  // ISAX_ULISSE_ENVELOPE_INDEX_HPP
