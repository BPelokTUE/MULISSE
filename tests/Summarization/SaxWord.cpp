#include "doctest.h"

#include "Summarization/SaxWord.hpp"

TEST_CASE("iSAX from PAA works") {
    vec<float> paa = {0.9, 2.3, -3.0, -1.1, -5.0, 5.3}, breakpoints = {-2.0, 0.0, 2.0};

    SaxWord sax(paa, 2, breakpoints);
    vec<SaxSymbolT> expected = {2, 3, 0, 1, 0, 3};

    for (size_t i = 0; i < paa.size(); ++i) {
        CHECK_EQ(sax[i], expected[i]);
    }
}
