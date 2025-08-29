#include "Index/Sax/BreakpointStrategy/FixedBreakpointStrategy.hpp"

#include "Index/Sax/SaxBreakpoints.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/InputStream.hpp"

FixedBreakpointStrategy::FixedBreakpointStrategy(const str &file) : m_breakpoints_file(file) {}

SaxBreakpoints FixedBreakpointStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    InputStream istream(m_breakpoints_file);

    vec<Real> breakpoints;
    Real breakpoint;
    auto &ifs = istream.get();
    while (ifs >> breakpoint) {
        breakpoints.push_back(breakpoint);
    }

    SaxSymbolT num_breakpoints = static_cast<SaxSymbolT>(breakpoints.size());
    if (alphabet_size > num_breakpoints + 1) {
        throw std::runtime_error("Requested alphabet size exceeds the number of breakpoints available.");
    }
    if (alphabet_size == num_breakpoints + 1) {
        return SaxBreakpoints(breakpoints);
    } else {  // alphabet_size < num_breakpoints + 1
        SaxSymbolT alphabet_ratio = static_cast<SaxSymbolT>((breakpoints.size() + 1) / alphabet_size);
        for (SaxSymbolT i = 1; i < alphabet_size; ++i) {
            breakpoints[i] = breakpoints[static_cast<SaxSymbolT>(i * alphabet_ratio - 1)];
        }
        return SaxBreakpoints(breakpoints);
    }
}
