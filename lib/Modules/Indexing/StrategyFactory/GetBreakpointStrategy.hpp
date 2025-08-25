#ifndef MODULES_INDEXING_GETBREAKPOINTSTRATEGY_HPP
#define MODULES_INDEXING_GETBREAKPOINTSTRATEGY_HPP

#include "Index/IndexParams.hpp"
#include "Util/Types/Pointers.hpp"

class ISaxBreakpointStrategy;

/**
 * @brief Get the ISaxBreakpointStrategy based on the provided SaxParams.
 * @param sax_params The parameters containing the number of bits and breakpoint strategy type.
 */
uptr<ISaxBreakpointStrategy> get_breakpoint_strategy(const SaxProperties &sax_params);

#endif  // MODULES_INDEXING_GETBREAKPOINTSTRATEGY_HPP
