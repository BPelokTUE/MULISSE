#include "doctest.h"

#include "Summarization/Paa.hpp"

TEST_CASE("PAA happy-flow works") {
    vec<float> ts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    unsigned segment_len = 2;

    vec<float> actual = paa(ts, segment_len);
    vec<float> expected = {1.5, 3.5, 5.5, 7.5, 9.5};
    CHECK_EQ(actual, expected);

    segment_len = 3;
    actual = paa(ts, segment_len);
    expected = {2, 5, 8};
    CHECK_EQ(actual, expected);

    segment_len = 11;
    actual = paa(ts, segment_len);
    expected = {};
    CHECK_EQ(actual, expected);
}
