#include "doctest.h"

#include "Summarization/Paa.hpp"

TEST_CASE("paa happy-flow works") {
    std::vector<float> ts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    unsigned segment_len = 2;

    std::vector<float> actual = paa(ts, segment_len);
    std::vector<float> expected = {1.5, 3.5, 5.5, 7.5, 9.5};

    CHECK_EQ(actual, expected);
}
