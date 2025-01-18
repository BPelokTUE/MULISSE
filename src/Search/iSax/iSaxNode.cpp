#include "Search/iSax/iSaxNode.hpp"

iSaxInternalNode::iSaxInternalNode(SaxSplitIndT split_ind) : m_split_ind(split_ind) {}

iSaxLeaf::iSaxLeaf(vec<FilePositionT> file_positions) : m_file_positions(file_positions) {}

iSaxSplittableLeaf::iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<UlisseEnvelope> envelopes)
    : iSaxLeaf(file_positions), m_envelopes(envelopes) {}
