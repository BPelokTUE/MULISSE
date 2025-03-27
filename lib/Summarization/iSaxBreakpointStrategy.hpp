#ifndef BREAKPOINT_STRATEGY_HPP
#define BREAKPOINT_STRATEGY_HPP

#include <boost/math/distributions/normal.hpp>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enum type for IiSaxBreakpointStrategy */
enum iSaxBreakpointStrategyType { EQUIPROBABLE };

DEFINE_ENUM_CONSTS_NO_EXTRA(iSaxBreakpointStrategyType, ISAX_BREAKPOINT_STRATEGY, false);

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

    /**
     * @brief Get the breakpoints for the given alphabet size
     * @param alphabet_size The size of the alphabet; assumed to be a power of two
     * @return Vector of breakpoints
     */
    virtual vec<Real> get_breakpoints(SaxSymbolT alphabet_size) const = 0;

    /**
     * @brief Adapt the breakpoints based on dataset statistics
     * @param mu Mean of the dataset entries
     * @param sigma Standard deviation of the dataset entries
     */
    virtual void adapt_to_dataset(Real mu, Real sigma) {}
};

/**
 * @brief Equiprobable breakpoints strategy
 *
 * Breakpoints strategy that returns breakpoints that divide the normal distribution into equal probability intervals
 */
class EquiprobableBreakpointStrategy : public IiSaxBreakpointStrategy {
   public:
    /**
     * @brief Constructor
     *
     * @param mean Mean of the normal distribution
     * @param standard_deviation Standard deviation of the normal distribution
     */
    EquiprobableBreakpointStrategy(Real mean = 0.0, Real standard_deviation = 1.0);

    vec<Real> get_breakpoints(SaxSymbolT alphabet_size) const override;

    void adapt_to_dataset(Real mu, Real sigma) override;

   private:
    boost::math::normal_distribution<Real> m_distribution;
};

#endif  // BREAKPOINT_STRATEGY_HPP
