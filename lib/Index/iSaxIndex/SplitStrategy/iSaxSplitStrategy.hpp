#ifndef ISAX_SPLIT_STRATEGY_HPP
#define ISAX_SPLIT_STRATEGY_HPP

#include <cstdlib>

#include "Index/Entry/EntryData.hpp"
#include "Index/iSaxIndex/SplittableISaxNode.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/SaxSplitIndex.hpp"

/** @brief Interface for iSAX split strategies */
template <typename T>
    requires DerivedFromEntryData<T>
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
