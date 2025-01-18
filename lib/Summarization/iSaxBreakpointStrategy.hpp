#ifndef BREAKPOINT_STRATEGY_HPP
#define BREAKPOINT_STRATEGY_HPP

#include <boost/math/distributions/normal.hpp>

#include "typedefs.hpp"

/**
 * @brief Interface for breakpoints strategies
 *
 * Interface for breakpoints strategies. Strategies should satisfy the doubling property,
 * i.e. doubling the size of the alphabet should result in a new set of breakpoints, such that
 * beta_old[i] = beta_new[2*i] for i = 0, 1, ..., alphabet_size_old - 1.
 */
class IiSaxBreakpointStrategy {
   public:
    virtual ~IiSaxBreakpointStrategy() = default;
    virtual vec<float> get_breakpoints(SaxSymbolT alphabet_size) const = 0;
};

class EquiprobableStrategy : public IiSaxBreakpointStrategy {
   public:
    EquiprobableStrategy(float standard_deviation = 1.0);

    vec<float> get_breakpoints(SaxSymbolT alphabet_size) const override;

   private:
    boost::math::normal_distribution<float> m_distribution;
};

#endif  // BREAKPOINT_STRATEGY_HPP
