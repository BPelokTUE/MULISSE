#ifndef ISAX_SPLIT_STRATEGY_HPP
#define ISAX_SPLIT_STRATEGY_HPP

#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enum for IiSaxSplitStrategy implementations */
enum iSaxSplitStrategyType { DOUBLE_ROUND_ROBIN, ENTROPY_MAXIMIZING };

DEFINE_ENUM_CONSTS_NO_EXTRA(iSaxSplitStrategyType, ISAX_SPLIT_STRATEGY, true);

/** @brief Interface for iSAX split strategies */
class IiSaxSplitStrategy {
   public:
    virtual ~IiSaxSplitStrategy() {}

    /**
     * @brief Get a channel and segment index to split on
     *
     * @param leaf The leaf to get the split index for
     * @return A channel and segment index to split on (see SaxSplitIndT)
     */
    virtual SaxSplitIndT get_split_ind(const iSaxSplittableLeaf *leaf, const vec<iSaxWord> &isax_mins) = 0;
};

/**
 * @brief Double round-robin split strategy
 *
 * Split strategy that selects the channel and segment indices using round-robin with separate counters
 * */
class DoubleRoundRobinStrategy : public IiSaxSplitStrategy {
   public:
    /**
     * @brief Constructor
     *
     * @param num_seg_per_channel Number of segments per channel
     * @param num_channels Number of channels
     */
    DoubleRoundRobinStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels);

    SaxSplitIndT get_split_ind(const iSaxSplittableLeaf *leaf, const vec<iSaxWord> &isax_mins) override;

   private:
    SaxSegIndT m_num_seg_per_channel, m_current_split = 0;
    MtsNumChannelsT m_num_channels, m_current_channel = 0;
};

class EntropyMaximizingStrategy : public IiSaxSplitStrategy {
   public:
    EntropyMaximizingStrategy(bool choose_min_num_bits_when_tied);

    SaxSplitIndT get_split_ind(const iSaxSplittableLeaf *leaf, const vec<iSaxWord> &isax_mins) override;

   private:
    bool m_choose_min_num_bits_when_tied;
};

#endif  // ISAX_SPLIT_STRATEGY_HPP
