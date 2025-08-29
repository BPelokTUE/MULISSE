#ifndef INDEX_SAX_SAXBREAKPOINTSTRATEGY_HPP
#define INDEX_SAX_SAXBREAKPOINTSTRATEGY_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"
#include "Util/Types/Vec.hpp"

class SaxBreakpoints;

/**
 * @brief Interface for breakpoints strategies
 *
 * Interface for breakpoints strategies. Strategies should satisfy:
 * (1) the returned vector should not include the first and last breakpoints, which are
 * assumed to be -inf and inf respectively
 * (2) doubling the size of the alphabet should result in a new set of breakpoints, such that
 * beta_old[i] = beta_new[2*i + 1] for i = 0, 1, ..., alphabet_size_old - 1.
 */
class ISaxBreakpointStrategy {
   public:
    virtual ~ISaxBreakpointStrategy() = default;

    /**
     * @brief Get the breakpoints for the given alphabet size
     * @param alphabet_size The size of the alphabet; assumed to be a power of two
     * @return Breakpoints
     */
    virtual SaxBreakpoints get_breakpoints(SaxSymbolT alphabet_size) const = 0;

    /**
     * @brief Adapt the breakpoints based on dataset statistics
     * @param mu Mean of the dataset entries
     * @param sigma Standard deviation of the dataset entries
     */
    virtual void adapt_to_dataset(Real mu, Real sigma) {}
};

#endif  // INDEX_SAX_SAXBREAKPOINTSTRATEGY_HPP
