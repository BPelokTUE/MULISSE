#include "doctest.h"

#include "Summarization/iSax.hpp"

TEST_CASE("iSAX from PAA works") {
    std::vector<float> paa = {0.9, 2.3, -3.0, -1.1, -5.0, 5.3}, breakpoints = {-2.0, 0.0, 2.0};

    iSaxWord isax(paa, 2, breakpoints);
    std::vector<unsigned> expected = {2, 3, 0, 1, 0, 3};

    for (unsigned i = 0; i < paa.size(); ++i) {
        CHECK_EQ(isax[i], expected[i]);
    }
}
