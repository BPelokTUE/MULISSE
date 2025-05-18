#ifndef INDEX_ISAXINDEX_SPLITSTRATEGY_ENTROPYMAXIMIZINGSPLITSTRATEGY_HPP
#define INDEX_ISAXINDEX_SPLITSTRATEGY_ENTROPYMAXIMIZINGSPLITSTRATEGY_HPP

#include "Index/iSaxIndex/SplitStrategy/iSaxSplitStrategy.hpp"

/**
 * @brief Entropy maximizing split strategy
 *
 * Split strategy that selects the segment that, when split, maximizes the entropy of
 */
template <typename T>
class EntropyMaximizingSplitStrategy : public IiSaxSplitStrategy<T> {
   public:
    /**
     * @brief Constructor
     * @param choose_min_num_bits_when_tied If true, when the score is tied, the segment with the smallest number of
     * bits is chosen
     */
    EntropyMaximizingSplitStrategy(bool choose_min_num_bits_when_tied);

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_words) override;

   private:
    bool m_choose_min_num_bits_when_tied;
};

#endif  // INDEX_ISAXINDEX_SPLITSTRATEGY_ENTROPYMAXIMIZINGSPLITSTRATEGY_HPP
