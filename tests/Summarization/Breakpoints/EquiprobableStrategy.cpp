#include "doctest/doctest.h"

#include "Summarization/iSaxBreakpointStrategy.hpp"

TEST_CASE("Equiprobable strategy works for standard normal") {
    EquiprobableStrategy strategy(1.0);
    auto breakpoints = strategy.get_breakpoints(4);
    CHECK(breakpoints.size() == 5);

    CHECK(breakpoints[0] == -std::numeric_limits<float>::infinity());
    CHECK(breakpoints[1] == doctest::Approx(-0.67448975));
    CHECK(breakpoints[2] == doctest::Approx(0.0));
    CHECK(breakpoints[3] == doctest::Approx(0.67448975));
    CHECK(breakpoints[4] == std::numeric_limits<float>::infinity());
}
