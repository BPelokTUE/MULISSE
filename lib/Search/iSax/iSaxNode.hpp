#ifndef ISAX_NODE_HPP
#define ISAX_NODE_HPP

#include "typedefs.hpp"

class iSaxNode {
   public:
    virtual ~iSaxNode() = default;
    virtual vec<FilePositionT> get_file_positions() const = 0;
    virtual SaxSplitIndT get_split_ind() const = 0;
    virtual bool is_leaf() const = 0;
};

#endif  // ISAX_NODE_HPP
