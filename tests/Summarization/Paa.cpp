#include <iostream>

#include "doctest/doctest.h"

#include "Summarization/Paa.hpp"

TEST_CASE("PAA happy-flow works") {
    vec<float> ts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint segment_len = 2;

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

TEST_CASE("get_paa_entries_normalized works") {
    uint segment_len = 3, l_min = 4, l_max = 7;
    iSaxPaaParams params = {segment_len, l_min, l_max};
    iSaxPaaGenerator generator(1, params);

    vec<vec<float>> uts = {{3, 7, 1.2, 3.7, 9.1, -3.5, -1.5, 0, 0.8}};
    vec<IndexEntry<Paa>> entries = generator.get_entries(uts, 0);

    for (auto entry : entries) {
        std::cout << entry.subsequence_position.start_pos << '\n';
        for (auto val : entry.mts_summary[0].paa_values) {
            std::cout << val << ' ';
        }
        std::cout << '\n';
    }
}
