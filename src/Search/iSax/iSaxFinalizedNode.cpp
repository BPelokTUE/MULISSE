#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Util/typedefs.hpp"

// iSaxFinalizedInternal

iSaxFinalizedInternal::iSaxFinalizedInternal(SaxSplitIndex split_ind, SaxSymbolT max_symbol_left,
                                             SaxSymbolT max_symbol_right, uptr<iSaxFinalizedNode> left,
                                             uptr<iSaxFinalizedNode> right)
    : m_split_ind(split_ind),
      m_max_symbol_left(max_symbol_left),
      m_max_symbol_right(max_symbol_right),
      m_left(std::move(left)),
      m_right(std::move(right)) {}

std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> iSaxFinalizedInternal::get_children() const {
    return {m_left.get(), m_right.get()};
}

pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedInternal::get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                             SaxNumBitsT symbol_num_bits) const {
    SaxNumBitsT shift = symbol_num_bits - split_num_bits - 1;
    assert(shift >= 0);
    return {m_max_symbol_left >> shift, m_max_symbol_right >> shift};
}

SaxSplitIndex iSaxFinalizedInternal::get_split_ind() const { return m_split_ind; }

vec<SubsequencePosition> iSaxFinalizedInternal::get_subsequence_positions() const { return {}; }

bool iSaxFinalizedInternal::is_leaf() const { return false; }

// iSaxFinalizedLeaf

iSaxFinalizedLeaf::iSaxFinalizedLeaf(vec<SubsequencePosition> subsequence_positions)
    : m_subsequence_positions(subsequence_positions) {}

std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> iSaxFinalizedLeaf::get_children() const {
    return {nullptr, nullptr};
};

pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedLeaf::get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                         SaxNumBitsT symbol_num_bits) const {
    return {-1, -1};
}

SaxSplitIndex iSaxFinalizedLeaf::get_split_ind() const { return {0, 0}; }

vec<SubsequencePosition> iSaxFinalizedLeaf::get_subsequence_positions() const { return m_subsequence_positions; }

bool iSaxFinalizedLeaf::is_leaf() const { return true; }
