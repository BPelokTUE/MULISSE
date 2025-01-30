#ifndef ISAX_SPLIT_STRATEGY_HPP
#define ISAX_SPLIT_STRATEGY_HPP

#include "typedefs.hpp"
#include "util.hpp"

/** @brief Enum for IiSaxSplitStrategy implementations */
enum iSaxSplitStrategyType { DOUBLE_ROUND_ROBIN };

/** @brief Map from strings to iSaxSplitStrategyType */
const umap<str, iSaxSplitStrategyType> STR_TO_ISAX_SPLIT_STRATEGY = {{"double_round_robin", DOUBLE_ROUND_ROBIN}};

/** @brief Vector of accepted strings for STR_TO_ISAX_SPLIT_STRATEGY */
const vec<str> ISAX_SPLIT_STRATEGY_STRS = get_keys(STR_TO_ISAX_SPLIT_STRATEGY);

/** @brief Interface for iSAX split strategies */
class IiSaxSplitStrategy {
   public:
    virtual ~IiSaxSplitStrategy() {}

    /**
     * @brief Get a channel and segment index to split on
     *
     * @return A channel and segment index to split on (see SaxSplitIndT)
     */
    virtual SaxSplitIndT get_split_ind() = 0;
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

    SaxSplitIndT get_split_ind() override;

   private:
    SaxSegIndT m_num_seg_per_channel, m_current_split = 0;
    MtsNumChannelsT m_num_channels, m_current_channel = 0;
};

#endif  // ISAX_SPLIT_STRATEGY_HPP
