#include "Index/Segmentation/Presence.hpp"

#include <doctest/doctest.h>

void check_presence_array(const PresenceArray &presence_array, const vec<size_t> &expected_presences,
                          size_t expected_sum) {
    auto presences = presence_array.get_presences();
    auto presence_sum = presence_array.get_presence_sum();

    CHECK_EQ(presence_sum, expected_sum);
    CHECK_EQ(presences.size(), expected_presences.size());

    for (size_t i = 0; i < expected_presences.size(); ++i) {
        CHECK_EQ(presences[i], expected_presences[i]);
    }
}

TEST_CASE("PresenceArray constructor works") {
    check_presence_array(PresenceArray(10, 30, 30, 30),
                         {0,   231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 210, 190, 171, 153, 136,
                          120, 105, 91,  78,  66,  55,  45,  36,  28,  21,  15,  10,  6,   3,   1,   0},
                         3850);

    check_presence_array(PresenceArray(3, 10, 12, 4), {0, 31, 31, 31, 27, 23, 19, 15, 11, 7, 3, 0}, 198);
}
