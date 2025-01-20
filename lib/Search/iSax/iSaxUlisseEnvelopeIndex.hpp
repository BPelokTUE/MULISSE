#ifndef ISAX_ULISSE_ENVELOPE_INDEX_HPP
#define ISAX_ULISSE_ENVELOPE_INDEX_HPP

#include <unordered_map>

#include "Search/IUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"

class iSaxUlisseEnvelopeIndex : IUlisseEnvelopeIndex {
   public:
    iSaxUlisseEnvelopeIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                            std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                            std::unique_ptr<IiSaxSplitStrategy> split_strategy);

    iSaxUlisseEnvelopeIndex() = default;

    ~iSaxUlisseEnvelopeIndex() = default;

    void insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) override;

    const iSaxNode *get_first_layer_node(const SaxWord &sax_min) const;

    vec<FilePositionT> search(vec<vec<float>> mts, const SearchOptions &search_options) const override;

   private:
    std::unordered_map<SaxWord, std::unique_ptr<iSaxNode>> m_first_layer;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    size_t m_leaf_capacity;
    std::unique_ptr<IiSaxBreakpointStrategy> m_breakpoint_strategy;
    vec<float> m_breakpoints;
    std::unique_ptr<IiSaxSplitStrategy> m_split_strategy;
};

#endif  // ISAX_ULISSE_ENVELOPE_INDEX_HPP
