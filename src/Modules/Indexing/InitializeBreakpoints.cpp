#include "Modules/Indexing/InitializeBreakpoints.hpp"

#include "Modules/Indexing/StrategyFactory/GetBreakpointStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"

void initialize_sax_breakpoints(const SaxParams &sax_params) {
    auto &RS = RunSettings::get_instance();
    if (!RS.breakpoints_set()) {
        auto breakpoint_strategy = get_breakpoint_strategy(sax_params);
        auto breakpoints = breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << sax_params.m_num_bits));
        RS.set_breakpoint_props({sax_params.m_num_bits, std::move(breakpoint_strategy), breakpoints});
    }
}
