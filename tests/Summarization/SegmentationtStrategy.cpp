#include <doctest/doctest.h>

#include "Summarization/SegmentationStrategy.hpp"

// Test cases for UniformSegmentationStrategy

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

// Test cases for AdaptiveSegmentationStrategy

TEST_CASE("AdaptiveSegmentationStrategy get_num_segments works") {
    for (uint pos_per_env : {0, 5, 9}) {
        auto ss = AdaptiveSegmentationStrategy(10, 30, 30, 10);
        CHECK_EQ(ss.get_num_segments(30), 10);
        CHECK_EQ(ss.get_num_segments(10), 5);
        CHECK_EQ(ss.get_num_segments(20), 9);
        CHECK_EQ(ss.get_num_segments(19), 8);

        ss = AdaptiveSegmentationStrategy(128, 2048, 2048, 16);
        CHECK_EQ(ss.get_num_segments(2048), 16);
        CHECK_EQ(ss.get_num_segments(1024), 13);
        CHECK_EQ(ss.get_num_segments(512), 9);
        CHECK_EQ(ss.get_num_segments(256), 5);
        CHECK_EQ(ss.get_num_segments(128), 2);

        ss = AdaptiveSegmentationStrategy(128, 2048, 3072, 16);
        CHECK_EQ(ss.get_num_segments(2048), 16);
        CHECK_EQ(ss.get_num_segments(1024), 12);
        CHECK_EQ(ss.get_num_segments(512), 7);
        CHECK_EQ(ss.get_num_segments(256), 4);
        CHECK_EQ(ss.get_num_segments(128), 2);
    }
}

TEST_CASE("AdaptiveSegmentationStrategy get_segment_len works") {
    auto ss = AdaptiveSegmentationStrategy(10, 30, 30, 10);
    CHECK_EQ(ss.get_segment_len(0), 2);
    CHECK_EQ(ss.get_segment_len(1), 2);
    CHECK_EQ(ss.get_segment_len(2), 2);
    CHECK_EQ(ss.get_segment_len(3), 2);
    CHECK_EQ(ss.get_segment_len(4), 2);
    CHECK_EQ(ss.get_segment_len(5), 2);
    CHECK_EQ(ss.get_segment_len(6), 2);
    CHECK_EQ(ss.get_segment_len(7), 3);
    CHECK_EQ(ss.get_segment_len(8), 3);
    CHECK_EQ(ss.get_segment_len(9), 10);

    ss = AdaptiveSegmentationStrategy(10, 30, 30, 10, 5);
    CHECK_EQ(ss.get_segment_len(0), 2);
    CHECK_EQ(ss.get_segment_len(1), 2);
    CHECK_EQ(ss.get_segment_len(2), 2);
    CHECK_EQ(ss.get_segment_len(3), 2);
    CHECK_EQ(ss.get_segment_len(4), 2);
    CHECK_EQ(ss.get_segment_len(5), 2);
    CHECK_EQ(ss.get_segment_len(6), 3);
    CHECK_EQ(ss.get_segment_len(7), 3);
    CHECK_EQ(ss.get_segment_len(8), 4);
    CHECK_EQ(ss.get_segment_len(9), 8);

    ss = AdaptiveSegmentationStrategy(128, 2048, 2048, 8);
    CHECK_EQ(ss.get_segment_len(0), 96);
    CHECK_EQ(ss.get_segment_len(1), 99);
    CHECK_EQ(ss.get_segment_len(2), 110);
    CHECK_EQ(ss.get_segment_len(3), 126);
    CHECK_EQ(ss.get_segment_len(4), 148);
    CHECK_EQ(ss.get_segment_len(5), 186);
    CHECK_EQ(ss.get_segment_len(6), 265);
    CHECK_EQ(ss.get_segment_len(7), 1018);

    ss = AdaptiveSegmentationStrategy(128, 2048, 3072, 8);
    CHECK_EQ(ss.get_segment_len(0), 117);
    CHECK_EQ(ss.get_segment_len(1), 122);
    CHECK_EQ(ss.get_segment_len(2), 135);
    CHECK_EQ(ss.get_segment_len(3), 153);
    CHECK_EQ(ss.get_segment_len(4), 178);
    CHECK_EQ(ss.get_segment_len(5), 219);
    CHECK_EQ(ss.get_segment_len(6), 300);
    CHECK_EQ(ss.get_segment_len(7), 824);
}

TEST_CASE("AdaptiveSegmentationStrategy get_type works") {
    auto ss = AdaptiveSegmentationStrategy(10, 30, 30, 10);
    CHECK_EQ(ss.get_type(), ADAPTIVE);

    ss = AdaptiveSegmentationStrategy(128, 512, 2048, 16);
    CHECK_EQ(ss.get_type(), ADAPTIVE);

    ss = AdaptiveSegmentationStrategy(500, 600, 1024, 7);
    CHECK_EQ(ss.get_type(), ADAPTIVE);
}
