#ifndef INDEX_SAX_BREAKPOINTSTRATEGY_FIXEDBREAKPOINTSTRATEGY_HPP
#define INDEX_SAX_BREAKPOINTSTRATEGY_FIXEDBREAKPOINTSTRATEGY_HPP

#include "Index/Sax/BreakpointStrategy/SaxBreakpointStrategy.hpp"

/**
 * @brief Fixed breakpoints strategy
 *
 * Use fixed breakpoints loaded from an external file.
 */
class FixedBreakpointStrategy : public ISaxBreakpointStrategy {
    str m_breakpoints_file;

   public:
    /**
     * @brief Constructor
     * @param file Path to the file containing the breakpoints
     */
    FixedBreakpointStrategy(const str &file);

    SaxBreakpoints get_breakpoints(SaxSymbolT alphabet_size) const override;
};

#endif  // INDEX_SAX_BREAKPOINTSTRATEGY_FIXEDBREAKPOINTSTRATEGY_HPP
