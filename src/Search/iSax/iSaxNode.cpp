#include <iostream>

#include "Search/iSax/iSaxNode.hpp"

std::pair<const iSaxNode *, const iSaxNode *> iSaxNode::get_children() const { return {nullptr, nullptr}; }
vec<FilePositionT> iSaxNode::get_file_positions() const { return {}; }
vec<UlisseEnvelope> iSaxNode::get_envelopes() const { return {}; }

iSaxInternalNode::iSaxInternalNode(SaxSplitIndT split_ind) : m_split_ind(split_ind) {}

std::pair<const iSaxNode *, const iSaxNode *> iSaxInternalNode::get_children() const {
    return {left.get(), right.get()};
}

iSaxLeaf::iSaxLeaf(vec<FilePositionT> file_positions) : m_file_positions(file_positions) {}

vec<FilePositionT> iSaxLeaf::get_file_positions() const { return m_file_positions; }

iSaxSplittableLeaf::iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<UlisseEnvelope> envelopes)
    : iSaxLeaf(file_positions), m_envelopes(envelopes) {}

vec<UlisseEnvelope> iSaxSplittableLeaf::get_envelopes() const { return m_envelopes; }
