#ifndef INDEX_ISAXINDEX_SPLITSTRATEGY_ULISSECLOSESTTOMEANSPLITSTRATEGY_HPP
#define INDEX_ISAXINDEX_SPLITSTRATEGY_ULISSECLOSESTTOMEANSPLITSTRATEGY_HPP

#include "Index/iSaxIndex/SplitStrategy/iSaxSplitStrategy.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Closest to mean split strategy in line with ULISSE implementation
 *
 * Split strategy that selects the last segment such that it's mean is closer to it's breakpoint than to the breakpoint
 * of the previously selected segment, and the breakpoint is within `max_std_dist` standard deviations of the mean
 */
template <typename T>
class UlisseClosestToMeanStrategy : public IiSaxSplitStrategy<T> {
   public:
    /** @brief Constructor */
    UlisseClosestToMeanStrategy(Real max_std_dist = 3.0);

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_words) override;

   private:
    Real m_max_std_dist;
};

#endif  // INDEX_ISAXINDEX_SPLITSTRATEGY_ULISSECLOSESTTOMEANSPLITSTRATEGY_HPP
