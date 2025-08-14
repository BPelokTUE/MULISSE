#ifndef UTIL_RUNSETTINGS_BREAKPOINTPROPERTIES_HPP
#define UTIL_RUNSETTINGS_BREAKPOINTPROPERTIES_HPP

#include "Index/Sax/BreakpointStrategy/SaxBreakpointStrategy.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

struct BreakpointProperties {
    SaxNumBitsT m_breakpoint_num_bits;
    uptr<ISaxBreakpointStrategy> m_breakpoint_strategy;
    vec<Real> m_breakpoints;
};

#endif  // UTIL_RUNSETTINGS_BREAKPOINTPROPERTIES_HPP
