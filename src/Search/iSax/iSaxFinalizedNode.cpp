#include "Search/iSax/iSaxFinalizedNode.hpp"

// iSaxFinalizedInternal

iSaxFinalizedInternal::iSaxFinalizedInternal(SaxSplitIndT split_ind, SaxSymbolT isax_max_left,
                                             SaxSymbolT isax_min_right, uptr<iSaxFinalizedNode> left,
                                             uptr<iSaxFinalizedNode> right)
    : m_split_ind(split_ind),
      m_max_symbol_left(isax_max_left),
      m_max_symbol_right(isax_min_right),
      m_left(std::move(left)),
      m_right(std::move(right)) {}

std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> iSaxFinalizedInternal::get_children() const {
    return {m_left.get(), m_right.get()};
}

pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedInternal::get_children_max_symbols() const {
    return {m_max_symbol_left, m_max_symbol_right};
}

SaxSplitIndT iSaxFinalizedInternal::get_split_ind() const { return m_split_ind; }

vec<FilePositionT> iSaxFinalizedInternal::get_file_positions() const { return {}; }

bool iSaxFinalizedInternal::is_leaf() const { return false; }

// iSaxFinalizedLeaf

iSaxFinalizedLeaf::iSaxFinalizedLeaf(vec<FilePositionT> file_positions) : m_file_positions(file_positions) {}

std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> iSaxFinalizedLeaf::get_children() const {
    return {nullptr, nullptr};
};

pair<SaxSymbolT, SaxSymbolT> iSaxFinalizedLeaf::get_children_max_symbols() const { return {-1, -1}; }

SaxSplitIndT iSaxFinalizedLeaf::get_split_ind() const { return {-1, -1}; }

vec<FilePositionT> iSaxFinalizedLeaf::get_file_positions() const { return m_file_positions; }

bool iSaxFinalizedLeaf::is_leaf() const { return true; }
