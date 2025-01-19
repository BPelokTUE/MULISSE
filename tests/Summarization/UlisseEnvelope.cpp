#include "doctest/doctest.h"

#include "Summarization/UlisseEnvelope.hpp"

TEST_CASE("ULISSE raw happy-flow works") {
    vec<float> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12};
    size_t ms_beg = 0;
    unsigned ms_per_env = 4;
    unsigned segment_len = 2;
    unsigned l_min = 3;
    unsigned l_max = 7;
    /*
    Expected envelopes:
    2.25, 2.25, 2.5,   3   -> (2.25, 3)
    2.5,  3,    5.5,   9   -> (2.5, 9)
    5.5,  9,    3.25, -0.5 -> (-0.5, 9)
    */

    auto envelope = ulisse_envelope_raw(ts, ms_beg, ms_per_env, segment_len, l_min, l_max);
    UlisseEnvelope expected = {{2.25, 2.5, -0.5}, {3, 9, 9}};

    CHECK_EQ(envelope.first.size(), expected.first.size());
    for (size_t i = 0; i < envelope.first.size(); ++i) {
        CHECK_EQ(envelope.first[i], doctest::Approx(expected.first[i]));
        CHECK_EQ(envelope.second[i], doctest::Approx(expected.second[i]));
    }
}

TEST_CASE("ULISSE normalized happy-flow works") {
    vec<float> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12};
    size_t ms_beg = 0;
    unsigned ms_per_env = 4;
    unsigned segment_len = 2;
    unsigned l_min = 3;
    unsigned l_max = 7;
    /*
    Expected envelopes:
    [-0.9486832980505138, -0.5449492609130661, -1.1111677990074318]
    [0.35355339059327384, 1.1835854998978794, 1.323448205074589]
    */

    auto envelope = ulisse_envelope_normalized(ts, ms_beg, ms_per_env, segment_len, l_min, l_max);
    UlisseEnvelope expected = {{-0.9486832980505138, -0.5449492609130661, -1.1111677990074318},
                               {0.35355339059327384, 1.1835854998978794, 1.323448205074589}};

    CHECK_EQ(envelope.first.size(), expected.first.size());
    for (size_t i = 0; i < envelope.first.size(); ++i) {
        CHECK_EQ(envelope.first[i], doctest::Approx(expected.first[i]));
        CHECK_EQ(envelope.second[i], doctest::Approx(expected.second[i]));
    }
}
