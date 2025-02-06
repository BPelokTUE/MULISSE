#ifndef ISAX_NODE_HPP
#define ISAX_NODE_HPP

#include "Util/typedefs.hpp"

/** @brief Base class for iSAX nodes */
class iSaxNode {
   public:
    virtual ~iSaxNode() = default;

    /**
     * @brief Get the file positions of the envelopes stored in the node if any
     *
     * @return Vector of file positions
     */
    virtual vec<FilePositionT> get_file_positions() const = 0;

    /**
     * @brief Get the segment and channel index to split the node on if any
     *
     * @return Channel and segment index (see SaxSplitIndT)
     */
    virtual SaxSplitIndT get_split_ind() const = 0;

    /**
     * @brief Check whether the node is a leaf or not
     *
     * @return `true` if the node is a leaf, `false` otherwise
     */
    virtual bool is_leaf() const = 0;
};

#endif  // ISAX_NODE_HPP
