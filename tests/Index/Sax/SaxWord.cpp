#include "Index/Sax/SaxWord.hpp"

#include <doctest/doctest.h>

TEST_CASE("iSAX from PAA works") {
    vec<Real> paa = {R(0.9), R(2.3), R(-3.0), R(-1.1), R(-5.0), R(5.3)}, breakpoints = {R(-2.0), R(0.0), R(2.0)};

    SaxWord sax(paa, breakpoints, 2);
    vec<SaxSymbolT> expected = {2, 3, 0, 1, 0, 3};

    for (SaxSegIndT i = 0; i < paa.size(); ++i) {
        CHECK_EQ(sax[i], expected[i]);
    }
}
