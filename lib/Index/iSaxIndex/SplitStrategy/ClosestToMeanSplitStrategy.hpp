#ifndef INDEX_ISAXINDEX_SPLITSTRATEGY_CLOSESTTOMEANSPLITSTRATEGY_HPP
#define INDEX_ISAXINDEX_SPLITSTRATEGY_CLOSESTTOMEANSPLITSTRATEGY_HPP

#include "Index/iSaxIndex/SplitStrategy/iSaxSplitStrategy.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Closest to mean split strategy
 *
 * Split strategy that selects the segment with mean closest to it's breakpoint
 */
template <typename T>
class ClosestToMeanSplitStrategy : public IiSaxSplitStrategy<T> {
   public:
    /** @brief Constructor */
    ClosestToMeanSplitStrategy(Real max_std_dist = 3.0);

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_words) override;

   private:
    Real m_max_std_dist;
};

#endif  // INDEX_ISAXINDEX_SPLITSTRATEGY_CLOSESTTOMEANSPLITSTRATEGY_HPP
