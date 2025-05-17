#ifndef INDEX_SAX_BREAKPOINTSTRATEGY_FIXEDBREAKPOINTSTRATEGY_HPP
#define INDEX_SAX_BREAKPOINTSTRATEGY_FIXEDBREAKPOINTSTRATEGY_HPP

#include "Index/Sax/BreakpointStrategy/SaxBreakpointStrategy.hpp"

/**
 * @brief Fixed breakpoints strategy
 *
 * Use fixed breakpoints loaded from an external file.
 */
class FixedBreakpointStrategy : public ISaxBreakpointStrategy {
   public:
    /**
     * @brief Constructor
     * @param file Path to plain text file containing the breakpoints
     */
    FixedBreakpointStrategy(const str& file);

    vec<Real> get_breakpoints(SaxSymbolT alphabet_size) const override;

   private:
    vec<Real> m_breakpoints;
};

#endif  // INDEX_SAX_BREAKPOINTSTRATEGY_FIXEDBREAKPOINTSTRATEGY_HPP
