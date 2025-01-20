#include <iostream>

#include "Search/iSax/iSaxNode.hpp"

// iSaxInternalNode

iSaxInternalNode::iSaxInternalNode(SaxSplitIndT split_ind) : m_split_ind(split_ind) {}

std::pair<const iSaxNode *, const iSaxNode *> iSaxInternalNode::get_children() const {
    return {left.get(), right.get()};
}

vec<FilePositionT> iSaxInternalNode::get_file_positions() const { return {}; }

vec<vec<UlisseEnvelope>> iSaxInternalNode::get_envelopes() const { return {}; }

SaxSplitIndT iSaxInternalNode::get_split_ind() const { return m_split_ind; }

bool iSaxInternalNode::is_leaf() const { return false; }

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
