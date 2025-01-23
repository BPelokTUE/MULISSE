#include <iostream>

#include "Search/iSax/iSaxNode.hpp"

// iSaxSplittableInternal

iSaxSplittableInternal::iSaxSplittableInternal(SaxSplitIndT split_ind) : m_split_ind(split_ind) {}

std::pair<const iSaxNode *, const iSaxNode *> iSaxSplittableInternal::get_children() const {
    return {left.get(), right.get()};
}

vec<FilePositionT> iSaxSplittableInternal::get_file_positions() const { return {}; }

vec<vec<UlisseEnvelope>> iSaxSplittableInternal::get_envelopes() const { return {}; }

SaxSplitIndT iSaxSplittableInternal::get_split_ind() const { return m_split_ind; }

bool iSaxSplittableInternal::is_leaf() const { return false; }

// iSaxLeaf

iSaxLeaf::iSaxLeaf(vec<FilePositionT> file_positions) : m_file_positions(file_positions) {}

std::pair<const iSaxNode *, const iSaxNode *> iSaxLeaf::get_children() const { return {nullptr, nullptr}; }

vec<FilePositionT> iSaxLeaf::get_file_positions() const { return m_file_positions; }

vec<vec<UlisseEnvelope>> iSaxLeaf::get_envelopes() const { return {}; }

SaxSplitIndT iSaxLeaf::get_split_ind() const { return {-1, -1}; }

bool iSaxLeaf::is_leaf() const { return true; }

// iSaxSplittableLeaf

iSaxSplittableLeaf::iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<vec<UlisseEnvelope>> envelopes)
    : iSaxLeaf(file_positions), m_envelopes(envelopes) {}

vec<vec<UlisseEnvelope>> iSaxSplittableLeaf::get_envelopes() const { return m_envelopes; }
