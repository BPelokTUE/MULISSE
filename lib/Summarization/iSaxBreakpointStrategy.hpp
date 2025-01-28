#ifndef BREAKPOINT_STRATEGY_HPP
#define BREAKPOINT_STRATEGY_HPP

#include <boost/math/distributions/normal.hpp>

#include "typedefs.hpp"
#include "util.hpp"

enum iSaxBreakpointStrategyType { EQUIPROBABLE };

const umap<str, iSaxBreakpointStrategyType> STR_TO_ISAX_BREAKPOINT_STRATEGY = {{"equiprobable", EQUIPROBABLE}};
const vec<str> ISAX_BREAKPOINT_STRATEGY_STRS = get_keys(STR_TO_ISAX_BREAKPOINT_STRATEGY);

/**
 * @brief Interface for breakpoints strategies
 *
 * Interface for breakpoints strategies. Strategies should satisfy:
 * (1) the returned vector should not include the first and last breakpoints, which are
 * assumed to be -inf and inf respectively
 * (2) doubling the size of the alphabet should result in a new set of breakpoints, such that
 * beta_old[i] = beta_new[2*i + 1] for i = 0, 1, ..., alphabet_size_old - 1.
 */
class IiSaxBreakpointStrategy {
   public:
    virtual ~IiSaxBreakpointStrategy() = default;
    virtual vec<float> get_breakpoints(SaxSymbolT alphabet_size) const = 0;
};

class EquiprobableBreakpointStrategy : public IiSaxBreakpointStrategy {
   public:
    EquiprobableBreakpointStrategy(float standard_deviation = 1.0);

    vec<float> get_breakpoints(SaxSymbolT alphabet_size) const override;

   private:
    boost::math::normal_distribution<float> m_distribution;
};

#endif  // BREAKPOINT_STRATEGY_HPP
