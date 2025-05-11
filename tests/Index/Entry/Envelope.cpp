#include "Index/Entry/Envelope.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>

#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

class EnvelopeTest {
   public:
    static vec<Envelope> get_raw_envelope(const vec<Real> &ts, EnvelopeParams env_params) {
        EnvelopeEntryGenerator generator(1, false, env_params);
        return generator.get_raw_envelopes(ts)[0];
    }

    static vec<Envelope> get_normalized_envelope(const vec<Real> &ts, EnvelopeParams env_params) {
        EnvelopeEntryGenerator generator(1, true, env_params);
        return generator.get_normalized_envelopes(ts)[0];
    }
};

TEST_CASE("raw envelope happy-flow works") {
    const vec<Real> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12, -9};
    uint pos_per_env = 4, l_min = 3, l_max = 7, segment_len = 2;

    fakeit::Mock<ISegmentationStrategy> segmentation_strategy_mock;
    fakeit::When(Method(segmentation_strategy_mock, get_num_segments)).AlwaysDo([segment_len](uint subs_len) {
        return subs_len / segment_len;
    });
    fakeit::When(Method(segmentation_strategy_mock, get_segment_len)).AlwaysReturn(segment_len);
    fakeit::When(Method(segmentation_strategy_mock, get_type)).AlwaysReturn(UNIFORM);

    fakeit::Mock<ILengthGroupSegmentationStrategy> lg_segmentation_strategy_mock;
    fakeit::When(Method(lg_segmentation_strategy_mock, get_const_segmentation_strategy))
        .AlwaysReturn(&segmentation_strategy_mock.get());
    fakeit::When(Method(lg_segmentation_strategy_mock, get_type)).AlwaysReturn(SINGLE);

    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::When(Method(run_settings_mock, get_length_group)).AlwaysReturn(0);
    fakeit::When(Method(run_settings_mock, get_lg_l_min)).AlwaysReturn(l_min);
    fakeit::When(Method(run_settings_mock, get_lg_l_max)).AlwaysReturn(l_max);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    /*
    Expected envelopes:
    2.25, 2.25, 2.5,   3   -> (2.25, 3)
    2.5,  3,    5.5,   9   -> (2.5, 9)
    5.5,  9,    3.25, -0.5 -> (-0.5, 9)
    ...
    */

    auto envelopes =
        EnvelopeTest::get_raw_envelope(ts, {l_min, l_max, pos_per_env, &lg_segmentation_strategy_mock.get()});
    vec<Envelope> expected = {
        {{R(2.25), R(2.5), R(-0.5)}, {3, 9, 9}},
        {{R(-0.5), R(-0.5), R(1.5)}, {9, R(7.25), R(7.25)}},
        {{R(7.25), -INF, -INF}, {R(7.25), INF, INF}},
    };

    REQUIRE(envelopes.size() == expected.size());
    for (size_t e = 0; e < envelopes.size(); ++e) {
        REQUIRE(envelopes[e].size() == expected[e].size());
        for (size_t i = 0; i < envelopes[e].size(); ++i) {
            REQUIRE(envelopes[e].m_lower[i] == doctest::Approx(expected[e].m_lower[i]));
            REQUIRE(envelopes[e].m_upper[i] == doctest::Approx(expected[e].m_upper[i]));
        }
    }
}

TEST_CASE("normalized envelope happy-flow works") {
    const vec<Real> ts = {1, 3.5, 1, 4, 2, 8, 10, -3.5, 2.5, 12, -9};
    uint ms_per_env = 4, segment_len = 2, l_min = 3, l_max = 7;

    fakeit::Mock<ISegmentationStrategy> segmentation_strategy_mock;
    fakeit::When(Method(segmentation_strategy_mock, get_num_segments)).AlwaysDo([segment_len](uint subs_len) {
        return subs_len / segment_len;
    });
    fakeit::When(Method(segmentation_strategy_mock, get_segment_len)).AlwaysReturn(segment_len);

    fakeit::Mock<ILengthGroupSegmentationStrategy> lg_segmentation_strategy_mock;
    fakeit::When(Method(lg_segmentation_strategy_mock, get_const_segmentation_strategy))
        .AlwaysReturn(&segmentation_strategy_mock.get());

    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::When(Method(run_settings_mock, get_length_group)).AlwaysReturn(0);
    fakeit::When(Method(run_settings_mock, get_lg_l_min)).AlwaysReturn(l_min);
    fakeit::When(Method(run_settings_mock, get_lg_l_max)).AlwaysReturn(l_max);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    /*
    Expected envelopes:
    [-0.9486832980505138, -0.5449492609130661, -1.1111677990074318]
    [0.35355339059327384, 1.1835854998978794, 1.323448205074589]

    [-0.6529396220694627, -1.1196572438256696, -0.24324618014776567]
    [0.9047619047619049, 0.6113337453508021, 0.5872853026473694]

    [0.6308598694087654, -inf, -inf]
    [0.6308598694087654, inf, inf]
    */
    auto envelopes =
        EnvelopeTest::get_normalized_envelope(ts, {l_min, l_max, ms_per_env, &lg_segmentation_strategy_mock.get()});

    vec<Envelope> expected = {{{R(-0.9486832980505138), R(-0.5449492609130661), R(-1.1111677990074318)},
                               {R(0.35355339059327384), R(1.1835854998978794), R(1.323448205074589)}},
                              {{R(-0.6529396220694627), R(-1.1196572438256696), R(-0.24324618014776567)},
                               {R(0.9047619047619049), R(0.6113337453508021), R(0.5872853026473694)}},
                              {{R(0.6308598694087654), -INF, -INF}, {R(0.6308598694087654), INF, INF}}};

    REQUIRE(envelopes.size() == expected.size());
    for (size_t e = 0; e < envelopes.size(); ++e) {
        REQUIRE(envelopes[e].size() == expected[e].size());
        for (size_t i = 0; i < envelopes[e].size(); ++i) {
            REQUIRE(envelopes[e].m_lower[i] == doctest::Approx(expected[e].m_lower[i]));
            REQUIRE(envelopes[e].m_upper[i] == doctest::Approx(expected[e].m_upper[i]));
        }
    }
}
