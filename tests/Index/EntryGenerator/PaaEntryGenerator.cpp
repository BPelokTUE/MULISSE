
#include "Index/EntryGenerator/PaaEntryGenerator.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "common.hpp"

TEST_CASE("get_paa_entries_normalized works") {
    uint segment_len = 3, l_min = 4, l_max = 7;

    fakeit::Mock<ISegmentationStrategy> segmentation_strategy_mock;
    fakeit::When(Method(segmentation_strategy_mock, get_num_segments)).AlwaysDo([segment_len](uint subs_len) {
        return subs_len / segment_len;
    });
    fakeit::When(Method(segmentation_strategy_mock, get_segment_len)).AlwaysReturn(segment_len);

    fakeit::Mock<IChannelSegmentationStrategy> ch_segmentation_strategy_mock;
    fakeit::When(Method(ch_segmentation_strategy_mock, get_const_segmentation_strategy))
        .AlwaysReturn(&segmentation_strategy_mock.get());

    fakeit::Mock<ILengthGroupSegmentationStrategy> lg_segmentation_strategy_mock;
    fakeit::When(Method(lg_segmentation_strategy_mock, get_const_ch_segmentation_strategy))
        .AlwaysReturn(&ch_segmentation_strategy_mock.get());

    LengthProperties length_props{
        .m_l_min = l_min,
        .m_l_max = l_max,
        .m_l_per_group = l_max - l_min + 1,
    };

    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::When(Method(run_settings_mock, get_length_props)).AlwaysReturn(length_props);

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

    require_paa_entries_equal(entries, expected);
}
