#include "Index/Entry/Paa.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>
#include <iostream>

#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

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
