#ifndef MODULES_INDEXING_INITIALIZEBREAKPOINTS_HPP
#define MODULES_INDEXING_INITIALIZEBREAKPOINTS_HPP

#include "Modules/Indexing/StrategyFactory/GetBreakpointStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"

void initialize_sax_breakpoints(const SaxParams &sax_params, SaxNumBitsT num_bits_limit) {
    auto &RS = RunSettings::get_instance();
    if (!RS.breakpoints_set()) {
        auto breakpoint_strategy = get_breakpoint_strategy(sax_params);
        auto breakpoints = breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << num_bits_limit));
        RS.set_breakpoint_props({num_bits_limit, std::move(breakpoint_strategy), breakpoints});
    }
}

#endif  // MODULES_INDEXING_INITIALIZEBREAKPOINTS_HPP
