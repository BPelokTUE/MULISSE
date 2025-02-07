#include "doctest/doctest.h"

#include "Summarization/Envelope.hpp"

TEST_CASE("ULISSE raw happy-flow works") {
    const vec<float> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12, -9};
    uint ms_per_env = 4;
    uint segment_len = 2;
    uint l_min = 3;
    uint l_max = 7;
    /*
    Expected envelopes:
    2.25, 2.25, 2.5,   3   -> (2.25, 3)
    2.5,  3,    5.5,   9   -> (2.5, 9)
    5.5,  9,    3.25, -0.5 -> (-0.5, 9)
    ...
    */
    auto envelopes = ulisse_envelope_raw(ts, {ms_per_env, segment_len, l_min, l_max});
    vec<Envelope> expected = {
        {{2.25, 2.5, -0.5}, {3, 9, 9}},
        {{-0.5, -0.5, 1.5}, {9, 7.25, 7.25}},
        {{7.25, -INF, -INF}, {7.25, INF, INF}},
    };

    REQUIRE(envelopes.size() == expected.size());
    for (size_t e = 0; e < envelopes.size(); ++e) {
        REQUIRE(envelopes[e].size() == expected[e].size());
        for (size_t i = 0; i < envelopes[e].size(); ++i) {
            REQUIRE(envelopes[e].lower[i] == doctest::Approx(expected[e].lower[i]));
            REQUIRE(envelopes[e].upper[i] == doctest::Approx(expected[e].upper[i]));
        }
    }
}

TEST_CASE("ULISSE normalized happy-flow works") {
    const vec<float> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12, -9};
    uint ms_per_env = 4;
    uint segment_len = 2;
    uint l_min = 3;
    uint l_max = 7;
    /*
    Expected envelopes:
    [-0.9486832980505138, -0.5449492609130661, -1.1111677990074318]
    [0.35355339059327384, 1.1835854998978794, 1.323448205074589]

    [-0.6529396220694627, -1.1196572438256696, -0.24324618014776567]
    [0.9047619047619049, 0.6113337453508021, 0.5872853026473694]

    [0.6308598694087654, -inf, -inf]
    [0.6308598694087654, inf, inf]
    */
    auto envelopes = ulisse_envelope_normalized(ts, {ms_per_env, segment_len, l_min, l_max});

    vec<Envelope> expected = {{{-0.9486832980505138, -0.5449492609130661, -1.1111677990074318},
                               {0.35355339059327384, 1.1835854998978794, 1.323448205074589}},
                              {{-0.6529396220694627, -1.1196572438256696, -0.24324618014776567},
                               {0.9047619047619049, 0.6113337453508021, 0.5872853026473694}},
                              {{0.6308598694087654, -INF, -INF}, {0.6308598694087654, INF, INF}}};

    REQUIRE(envelopes.size() == expected.size());
    for (size_t e = 0; e < envelopes.size(); ++e) {
        REQUIRE(envelopes[e].size() == expected[e].size());
        for (size_t i = 0; i < envelopes[e].size(); ++i) {
            REQUIRE(envelopes[e].lower[i] == doctest::Approx(expected[e].lower[i]));
            REQUIRE(envelopes[e].upper[i] == doctest::Approx(expected[e].upper[i]));
        }
    }
}
