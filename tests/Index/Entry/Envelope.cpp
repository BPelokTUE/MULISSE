#include "Index/Entry/Envelope.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>

TEST_CASE("merge works") {
    Envelope env1({1, 8, 3}, {4, 11, 6});
    Envelope env2({7, 2, 9}, {10, 5, 12});
    env1.merge(env2);
    Envelope expected({1, 2, 3}, {10, 11, 12});
    REQUIRE(env1 == expected);
}
