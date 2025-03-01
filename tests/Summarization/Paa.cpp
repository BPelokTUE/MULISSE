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
    PaaEntryGenerator generator(1, params);

    vec<vec<float>> uts = {{3, 7, 1.2, 3.7, 9.1, -3.5, -1.5, 0, 0.8}};
    vec<IndexEntry<Paa>> entries = generator.get_entries(uts, 0);
    vec<IndexEntry<Paa>> expected = {
        {{0, 0, 4}, {{{0.00396971}}}},
        {{0, 0, 5}, {{{-0.373683}}}},
        {{0, 1, 4}, {{{-0.423736}}}},
        {{0, 0, 6}, {{{0.0782954, -0.0782955}}}},
        {{0, 1, 5}, {{{0.105442}}}},
        {{0, 2, 4}, {{{0.449213}}}},
        {{0, 0, 7}, {{{0.247292, 0.0936011}}}},
        {{0, 1, 6}, {{{0.292186, -0.292186}}}},
        {{0, 2, 5}, {{{0.653408}}}},
        {{0, 3, 4}, {{{0.235}}}},
        {{0, 1, 7}, {{{0.397995, -0.217601}}}},
        {{0, 2, 6}, {{{0.779816, -0.779816}}}},
        {{0, 3, 5}, {{{0.346383}}}},
        {{0, 4, 4}, {{{0.0708175}}}},
        {{0, 2, 7}, {{{0.867058, -0.813973}}}},
        {{0, 3, 6}, {{{0.409657, -0.409657}}}},
        {{0, 4, 5}, {{{0.089585}}}},
        {{0, 5, 4}, {{{-0.376514}}}},
    };

    REQUIRE(entries.size() == expected.size());
    for (size_t i = 0; i < entries.size(); ++i) {
        REQUIRE(entries[i].subsequence_info == expected[i].subsequence_info);
        REQUIRE(entries[i].mts_summary.size() == expected[i].mts_summary.size());
        for (size_t j = 0; j < entries[i].mts_summary.size(); ++j) {
            REQUIRE(entries[i].mts_summary[j].size() == expected[i].mts_summary[j].size());
            for (size_t k = 0; k < entries[i].mts_summary[j].size(); ++k) {
                REQUIRE_EQ(entries[i].mts_summary[j].paa_values[k],
                           doctest::Approx(expected[i].mts_summary[j].paa_values[k]).epsilon(1e-5));
            }
        }
    }
}
