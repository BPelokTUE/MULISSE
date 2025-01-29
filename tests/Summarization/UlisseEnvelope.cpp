#include "doctest/doctest.h"

#include "Summarization/UlisseEnvelope.hpp"

TEST_CASE("ULISSE raw happy-flow works") {
    const vec<float> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12, -9};
    unsigned ms_per_env = 4;
    unsigned segment_len = 2;
    unsigned l_min = 3;
    unsigned l_max = 7;
    /*
    Expected envelopes:
    2.25, 2.25, 2.5,   3   -> (2.25, 3)
    2.5,  3,    5.5,   9   -> (2.5, 9)
    5.5,  9,    3.25, -0.5 -> (-0.5, 9)
    ...
    */

    auto envelopes = ulisse_envelope_raw(ts, {ms_per_env, segment_len, l_min, l_max});
    float inf = std::numeric_limits<float>::max(), neg_inf = std::numeric_limits<float>::min();
    vec<Envelope> expected = {
        {{2.25, 2.5, -0.5}, {3, 9, 9}},
        {{-0.5, -0.5, 1.5}, {9, 7.25, 7.25}},
        {{7.25, inf, inf}, {7.25, neg_inf, neg_inf}},
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
    const vec<float> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12};
    std::span<const float> ts_view(ts);
    unsigned ms_per_env = 4;
    unsigned segment_len = 2;
    unsigned l_min = 3;
    unsigned l_max = 7;
    /*
    Expected envelopes:
    [-0.9486832980505138, -0.5449492609130661, -1.1111677990074318]
    [0.35355339059327384, 1.1835854998978794, 1.323448205074589]
    */

    auto envelope = ulisse_envelope_normalized(ts_view, {ms_per_env, segment_len, l_min, l_max});
    Envelope expected = {{-0.9486832980505138, -0.5449492609130661, -1.1111677990074318},
                         {0.35355339059327384, 1.1835854998978794, 1.323448205074589}};

    CHECK_EQ(envelope.size(), expected.size());
    for (size_t i = 0; i < envelope.size(); ++i) {
        CHECK_EQ(envelope.lower[i], doctest::Approx(expected.lower[i]));
        CHECK_EQ(envelope.upper[i], doctest::Approx(expected.upper[i]));
    }
}
