#include <iostream>

#include <doctest/doctest.h>

#include "Summarization/Paa.hpp"

TEST_CASE("PAA happy-flow works") {
    vec<Real> ts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint segment_len = 2;

    vec<Real> actual = paa(ts, segment_len);
    vec<Real> expected = {R(1.5), R(3.5), R(5.5), R(7.5), R(9.5)};
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

    vec<vec<Real>> uts = {{3, 7, R(1.2), R(3.7), R(9.1), R(-3.5), R(-1.5), 0, R(0.8)}};
    vec<IndexEntry<Paa>> entries = generator.get_entries(uts, 0)[0];
    vec<IndexEntry<Paa>> expected = {
        {{0, 0, 4}, {{{R(0.00396971), R(0.0), R(0.0)}}}},     {{0, 0, 5}, {{{R(-0.373683), R(0.0), R(0.0)}}}},
        {{0, 1, 4}, {{{R(-0.423736), R(0.0), R(0.0)}}}},      {{0, 0, 6}, {{{R(0.0782954), R(-0.0782955), R(0.0)}}}},
        {{0, 1, 5}, {{{R(0.105442), R(0.0), R(0.0)}}}},       {{0, 2, 4}, {{{R(0.449213), R(0.0), R(0.0)}}}},
        {{0, 0, 7}, {{{R(0.247292), R(0.0936011), R(0.0)}}}}, {{0, 1, 6}, {{{R(0.292186), R(-0.292186), R(0.0)}}}},
        {{0, 2, 5}, {{{R(0.653408), R(0.0), R(0.0)}}}},       {{0, 3, 4}, {{{R(0.235), R(0.0), R(0.0)}}}},
        {{0, 1, 7}, {{{R(0.397995), R(-0.217601), R(0.0)}}}}, {{0, 2, 6}, {{{R(0.779816), R(-0.779816), R(0.0)}}}},
        {{0, 3, 5}, {{{R(0.346383), R(0.0), R(0.0)}}}},       {{0, 4, 4}, {{{R(0.0708175), R(0.0), R(0.0)}}}},
        {{0, 2, 7}, {{{R(0.867058), R(-0.813973), R(0.0)}}}}, {{0, 3, 6}, {{{R(0.409657), R(-0.409657), R(0.0)}}}},
        {{0, 4, 5}, {{{R(0.089585), R(0.0), R(0.0)}}}},       {{0, 5, 4}, {{{R(-0.376514), R(0.0), R(0.0)}}}},
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
