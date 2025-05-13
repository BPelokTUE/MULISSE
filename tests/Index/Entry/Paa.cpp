#include "Index/Entry/Paa.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>
#include <iostream>

#include "Index/EntryGenerator/PaaEntryGenerator.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

TEST_CASE("PAA happy-flow works") {
    vec<Real> ts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint segment_len = 2;  // l_max = 10
    fakeit::Mock<ISegmentationStrategy> segmentation_strategy_mock;
    fakeit::When(Method(segmentation_strategy_mock, get_num_segments)).AlwaysDo([&segment_len](uint subs_len) {
        return subs_len / segment_len;
    });
    fakeit::When(Method(segmentation_strategy_mock, get_segment_len)).AlwaysReturn(segment_len);

    vec<Real> actual = paa(ts, &segmentation_strategy_mock.get());
    vec<Real> expected = {R(1.5), R(3.5), R(5.5), R(7.5), R(9.5)};
    CHECK_EQ(actual, expected);

    segment_len = 3;
    fakeit::When(Method(segmentation_strategy_mock, get_segment_len)).AlwaysReturn(segment_len);

    actual = paa(ts, &segmentation_strategy_mock.get());
    expected = {2, 5, 8};
    CHECK_EQ(actual, expected);

    segment_len = 11;
    fakeit::When(Method(segmentation_strategy_mock, get_segment_len)).AlwaysReturn(segment_len);

    actual = paa(ts, &segmentation_strategy_mock.get());
    expected = {};
    CHECK_EQ(actual, expected);
}

TEST_CASE("get_paa_entries_normalized works") {
    uint segment_len = 3, l_min = 4, l_max = 7;

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

    PaaParams params = {l_min, l_max, &lg_segmentation_strategy_mock.get()};
    PaaEntryGenerator generator(params);

    vec<vec<Real>> uts = {{3, 7, R(1.2), R(3.7), R(9.1), R(-3.5), R(-1.5), 0, R(0.8)}};
    vec<IndexEntry<Paa>> entries = generator.get_entries(uts, 0)[0];
    vec<IndexEntry<Paa>> expected = {
        {{0, 0, 4}, {{{R(0.00396971), R(0.0)}}}},     {{0, 0, 5}, {{{R(-0.373683), R(0.0)}}}},
        {{0, 1, 4}, {{{R(-0.423736), R(0.0)}}}},      {{0, 0, 6}, {{{R(0.0782954), R(-0.0782955)}}}},
        {{0, 1, 5}, {{{R(0.105442), R(0.0)}}}},       {{0, 2, 4}, {{{R(0.449213), R(0.0)}}}},
        {{0, 0, 7}, {{{R(0.247292), R(0.0936011)}}}}, {{0, 1, 6}, {{{R(0.292186), R(-0.292186)}}}},
        {{0, 2, 5}, {{{R(0.653408), R(0.0)}}}},       {{0, 3, 4}, {{{R(0.235), R(0.0)}}}},
        {{0, 1, 7}, {{{R(0.397995), R(-0.217601)}}}}, {{0, 2, 6}, {{{R(0.779816), R(-0.779816)}}}},
        {{0, 3, 5}, {{{R(0.346383), R(0.0)}}}},       {{0, 4, 4}, {{{R(0.0708175), R(0.0)}}}},
        {{0, 2, 7}, {{{R(0.867058), R(-0.813973)}}}}, {{0, 3, 6}, {{{R(0.409657), R(-0.409657)}}}},
        {{0, 4, 5}, {{{R(0.089585), R(0.0)}}}},       {{0, 5, 4}, {{{R(-0.376514), R(0.0)}}}},
    };

    REQUIRE(entries.size() == expected.size());
    for (size_t i = 0; i < entries.size(); ++i) {
        REQUIRE(entries[i].m_subs_info == expected[i].m_subs_info);
        REQUIRE(entries[i].m_mts_summary.size() == expected[i].m_mts_summary.size());
        for (size_t j = 0; j < entries[i].m_mts_summary.size(); ++j) {
            REQUIRE(entries[i].m_mts_summary[j].size() == expected[i].m_mts_summary[j].size());
            for (size_t k = 0; k < entries[i].m_mts_summary[j].size(); ++k) {
                REQUIRE_EQ(entries[i].m_mts_summary[j].m_paa_values[k],
                           doctest::Approx(expected[i].m_mts_summary[j].m_paa_values[k]).epsilon(1e-5));
            }
        }
    }
}
