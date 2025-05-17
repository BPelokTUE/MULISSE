#ifndef MODULES_INDEXING_GETBREAKPOINTSTRATEGY_HPP
#define MODULES_INDEXING_GETBREAKPOINTSTRATEGY_HPP

#include "Index/IndexParams.hpp"
#include "Index/Sax/BreakpointStrategy/SaxBreakpointStrategy.hpp"
#include "Util/Types/Pointers.hpp"

uptr<ISaxBreakpointStrategy> get_breakpoint_strategy(const SaxParams &sax_params);

#endif  // MODULES_INDEXING_GETBREAKPOINTSTRATEGY_HPP
