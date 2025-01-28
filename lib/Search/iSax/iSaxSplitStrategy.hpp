#ifndef ISAX_SPLIT_STRATEGY_HPP
#define ISAX_SPLIT_STRATEGY_HPP

#include "typedefs.hpp"
#include "util.hpp"

enum iSaxSplitStrategyType { DOUBLE_ROUND_ROBIN };

const umap<str, iSaxSplitStrategyType> STR_TO_ISAX_SPLIT_STRATEGY = {{"double_round_robin", DOUBLE_ROUND_ROBIN}};
const vec<str> ISAX_SPLIT_STRATEGY_STRS = get_keys(STR_TO_ISAX_SPLIT_STRATEGY);

class IiSaxSplitStrategy {
   public:
    virtual ~IiSaxSplitStrategy() {}
    virtual SaxSplitIndT get_split_ind() = 0;
};

class DoubleRoundRobinStrategy : public IiSaxSplitStrategy {
   public:
    DoubleRoundRobinStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels);

    SaxSplitIndT get_split_ind() override;

   private:
    SaxSegIndT m_num_seg_per_channel, m_current_split = 0;
    MtsNumChannelsT m_num_channels, m_current_channel = 0;
};

#endif  // ISAX_SPLIT_STRATEGY_HPP
