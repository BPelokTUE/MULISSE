#ifndef INDEX_SAX_BREAKPOINTSTRATEGY_EQUIPROBABLEBREAKPOINTSTRATEGY_HPP
#define INDEX_SAX_BREAKPOINTSTRATEGY_EQUIPROBABLEBREAKPOINTSTRATEGY_HPP

#include <boost/math/distributions/normal.hpp>

#include "Index/Sax/BreakpointStrategy/SaxBreakpointStrategy.hpp"

/**
 * @brief Equiprobable breakpoints strategy
 *
 * Breakpoints strategy that returns breakpoints that divide the normal distribution into equal probability intervals
 */
class EquiprobableBreakpointStrategy : public ISaxBreakpointStrategy {
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

#endif  // INDEX_SAX_BREAKPOINTSTRATEGY_EQUIPROBABLEBREAKPOINTSTRATEGY_HPP
