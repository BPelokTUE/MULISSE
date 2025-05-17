#include "Modules/Indexing/StrategyFactory/GetBreakpointStrategy.hpp"

#include "Index/Sax/BreakpointStrategy/EquiprobableBreakpointStrategy.hpp"
#include "Index/Sax/BreakpointStrategy/FixedBreakpointStrategy.hpp"

uptr<ISaxBreakpointStrategy> get_breakpoint_strategy(const SaxParams &sax_params) {
    switch (sax_params.m_breakpoint_strategy_type) {
        case EQUIPROBABLE:
            return std::make_unique<EquiprobableBreakpointStrategy>();
        case FIXED:
            return std::make_unique<FixedBreakpointStrategy>(sax_params.m_breakpoints_file);
    }
    return nullptr;
}
