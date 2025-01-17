#ifndef ISAX_NODE_HPP
#define ISAX_NODE_HPP

#include "typedefs.hpp"

class iSaxNode {};

class iSaxInternalNode {
    iSaxSplitIndT split_ind;
    iSaxNode *left = nullptr, *right = nullptr;
};

class iSaxLeaf : iSaxNode {
    uint64_t file_position;
};

#endif  // ISAX_NODE_HPP
