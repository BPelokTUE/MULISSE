#include "doctest/doctest.h"

#include "Summarization/iSaxBreakpointStrategy.hpp"

TEST_CASE("Equiprobable strategy works for standard normal") {
    EquiprobableBreakpointStrategy strategy(0.0, 1.0);
    auto breakpoints = strategy.get_breakpoints(4);
    CHECK(breakpoints.size() == 3);

    CHECK(breakpoints[0] == doctest::Approx(-0.67448975));
    CHECK(breakpoints[1] == doctest::Approx(0.0));
    CHECK(breakpoints[2] == doctest::Approx(0.67448975));
}
