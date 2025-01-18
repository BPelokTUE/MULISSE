#include "doctest.h"

#include "Summarization/iSaxWord.hpp"

TEST_CASE("split symbol happy-flow works") {
    iSaxWord isax({1, 2, 0, 3, 1, 2}, 2);

    auto [left, right] = isax.split(2);

    for (auto i : {0, 1, 3, 4, 5}) {
        CHECK(left[i] == isax[i]);
        CHECK(right[i] == isax[i]);

        CHECK(left.get_num_bits(i) == isax.get_num_bits(i));
        CHECK(right.get_num_bits(i) == isax.get_num_bits(i));
    }

    CHECK(isax[2] == 0);
    CHECK(left[2] == 0);
    CHECK(right[2] == 1);

    CHECK(isax.get_num_bits(2) == 2);
    CHECK(left.get_num_bits(2) == 3);
    CHECK(right.get_num_bits(2) == 3);
}
