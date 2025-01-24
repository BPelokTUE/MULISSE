#include <iostream>

#include "Search/iSax/iSaxSplittableNode.hpp"

// iSaxSplittableInternal

iSaxSplittableInternal::iSaxSplittableInternal(SaxSplitIndT split_ind) : m_split_ind(split_ind) {}

std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> iSaxSplittableInternal::get_children() const {
    return {left.get(), right.get()};
}

SaxSplitIndT iSaxSplittableInternal::get_split_ind() const { return m_split_ind; }

vec<FilePositionT> iSaxSplittableInternal::get_file_positions() const { return {}; }

vec<vec<UlisseEnvelope>> iSaxSplittableInternal::get_envelopes() const { return {}; };

bool iSaxSplittableInternal::is_leaf() const { return false; }

std::unique_ptr<iSaxFinalizedNode> iSaxSplittableInternal::finalize() const { return nullptr; };

// iSaxSplittableLeaf

iSaxSplittableLeaf::iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<vec<UlisseEnvelope>> envelopes)
    : m_file_positions(file_positions), m_envelopes(envelopes) {}

std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> iSaxSplittableLeaf::get_children() const {
    return {nullptr, nullptr};
};

SaxSplitIndT iSaxSplittableLeaf::get_split_ind() const { return {-1, -1}; }

vec<FilePositionT> iSaxSplittableLeaf::get_file_positions() const { return m_file_positions; }

bool iSaxSplittableLeaf::is_leaf() const { return true; }

vec<vec<UlisseEnvelope>> iSaxSplittableLeaf::get_envelopes() const { return m_envelopes; }

std::unique_ptr<iSaxFinalizedNode> iSaxSplittableLeaf::finalize() const { return nullptr; };
