#include "Index/EntryGenerator/SaxMergingPaaEntryGenerator.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>

#include "Util/RunSettings/RunSettings.hpp"
#include "common.hpp"

TEST_CASE("get_entries works") {
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

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), R(0.0), R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;
    breakpoint_props_mock.m_breakpoints = breakpoints_mock;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    PaaParams params = {l_min, l_max, &lg_segmentation_strategy_mock.get()};
    SaxMergingPaaEntryGenerator generator(params, 3);

    vec<vec<Real>> uts = {{3, 7, R(1.2), R(3.7), R(9.1), R(-3.5), R(-1.5), 0, R(0.8)}};
    vec<IndexEntry<Paa>> entries = generator.get_entries(uts, 0)[0];
    vec<IndexEntry<Paa>> expected = {
        {{0, 0, 7}, {{{R(0.247292), R(0.0936011)}}}}, {{0, 0, 9}, {{{R(-0.373683), R(0.0)}}}},
        {{0, 0, 9}, {{{R(0.00396971), R(0.0)}}}},     {{0, 1, 4}, {{{R(-0.423736), R(0.0)}}}},
        {{0, 2, 5}, {{{R(0.449213), R(0.0)}}}},       {{0, 2, 7}, {{{R(0.779816), R(-0.779816)}}}},
        {{0, 3, 6}, {{{R(0.409657), R(-0.409657)}}}},
    };

    require_sorted_paa_entries_equal(entries, expected);
}
