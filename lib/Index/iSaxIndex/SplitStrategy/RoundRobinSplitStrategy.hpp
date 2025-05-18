#ifndef INDEX_ISAXINDEX_SPLITSTRATEGY_ROUNDROBINSPLITSTRATEGY_HPP
#define INDEX_ISAXINDEX_SPLITSTRATEGY_ROUNDROBINSPLITSTRATEGY_HPP

#include "Index/iSaxIndex/SplitStrategy/iSaxSplitStrategy.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Double round-robin split strategy
 *
 * Split strategy that selects the channel and segment indices using round-robin with separate counters
 * */
template <typename T>
class RoundRobinSplitStrategy : public IiSaxSplitStrategy<T> {
   public:
    /**
     * @brief Constructor
     * @param num_seg_per_channel Number of segments per channel
     * @param num_channels Number of channels
     */
    RoundRobinSplitStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels);

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_mins) override;

   private:
    SaxSegIndT m_num_seg_per_channel, m_current_split = 0;
    MtsNumChannelsT m_num_channels, m_current_channel = 0;
};

#endif  // INDEX_ISAXINDEX_SPLITSTRATEGY_ROUNDROBINSPLITSTRATEGY_HPP
