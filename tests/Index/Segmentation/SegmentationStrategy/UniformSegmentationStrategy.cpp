#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"

#include <doctest/doctest.h>

TEST_CASE("UniformSegmentationStrategy get_num_segments works") {
    auto ss = UniformSegmentationStrategy(100, 10);
    CHECK_EQ(ss.get_num_segments(100), 10);
    CHECK_EQ(ss.get_num_segments(99), 9);
    CHECK_EQ(ss.get_num_segments(53), 5);
}

TEST_CASE("UniformSegmentationStrategy get_segment_len works") {
    auto ss = UniformSegmentationStrategy(100, 10);
    CHECK_EQ(ss.get_segment_len(0), 10);
    CHECK_EQ(ss.get_segment_len(1), 10);
    CHECK_EQ(ss.get_segment_len(9), 10);

    ss = UniformSegmentationStrategy(11, 2);
    CHECK_EQ(ss.get_segment_len(0), 5);
    CHECK_EQ(ss.get_segment_len(1), 5);
}

TEST_CASE("UniformSegmentationStrategy get_type works") {
    auto ss = UniformSegmentationStrategy(100, 10);
    CHECK_EQ(ss.get_type(), UNIFORM);

    ss = UniformSegmentationStrategy(11, 2);
    CHECK_EQ(ss.get_type(), UNIFORM);
}
