#ifndef ISAX_SPLIT_STRATEGY_HPP
#define ISAX_SPLIT_STRATEGY_HPP

#include "Util/Types/Vec.hpp"

template <typename T>
class iSaxSplittableLeaf;

class iSaxWord;

class SaxSplitIndex;

/** @brief Interface for iSAX split strategies */
template <typename T>
class IiSaxSplitStrategy {
   public:
    virtual ~IiSaxSplitStrategy() {}

    /**
     * @brief Get a channel and segment index to split on
     * @param leaf The leaf to get the split index for
     * @return A channel and segment index to split on (see SaxSplitIndex)
     */
    virtual SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_mins) = 0;
};

#endif  // ISAX_SPLIT_STRATEGY_HPP
