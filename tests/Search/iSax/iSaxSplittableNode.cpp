#include "doctest/doctest.h"
#include "fakeit/fakeit.hpp"

#include "Search/iSax/iSaxSplittableNode.hpp"

TEST_CASE("iSAX leaf finalization works") {
    vec<SubsequencePosition> subsequence_positions{{51, 100}, {26, 21}, {6, 387}};
    vec<vec<Envelope>> envelopes = {{{{-1.5, 2.3}, {-0.1, 4.9}}, {{-7.9, 0.5}, {-3.3, 2.7}}},
                                    {{{-2.3, 3.6}, {0.6, 9.7}}, {{-8.1, -0.5}, {-1.5, 6.3}}},
                                    {{{-1.9, 1.9}, {1.9, 7.1}}, {{-10, -0.9}, {-9, 1.9}}}};

    iSaxSplittableLeaf leaf(subsequence_positions, envelopes);

    vec<SaxNumBitsT> num_bits = {3, 2};
    iSaxWordSettings isax_word_settings = {num_bits, 3, {-7, -3, -1, 1, 4, 9, 10}};

    auto [finalized, isax_max] = leaf.finalize(isax_word_settings);

    REQUIRE(finalized->is_leaf());
    REQUIRE(finalized->get_subsequence_positions() == subsequence_positions);
    REQUIRE(isax_max.size() == 2);
    REQUIRE(isax_max[0] == iSaxWord({4, 6}, num_bits, 3));
    REQUIRE(isax_max[1] == iSaxWord({2, 5}, num_bits, 3));
}

TEST_CASE("iSAX internal finalization works") {
    fakeit::Mock<iSaxSplittableNode> left, right;
    fakeit::Mock<iSaxFinalizedNode> fin_left, fin_right;

    vec<SaxNumBitsT> num_bits = {1, 2, 2};
    vec<iSaxWord> isax_max_left = {iSaxWord({2, 2, 1}, num_bits, 2), iSaxWord({3, 1, 2}, num_bits, 2)},
                  isax_max_right = {iSaxWord({1, 1, 3}, num_bits, 2), iSaxWord({3, 1, 3}, num_bits, 2)};

    fakeit::When(Method(left, finalize)).Return({std::unique_ptr<iSaxFinalizedNode>(&fin_left.get()), isax_max_left});
    fakeit::When(Method(right, finalize))
        .Return({std::unique_ptr<iSaxFinalizedNode>(&fin_right.get()), isax_max_right});

    SaxSplitIndex split_ind{1, 0};
    iSaxSplittableInternal internal(split_ind, &left.get(), &right.get());

    iSaxWordSettings isax_word_settings = {num_bits, 2, {-3, 0, 3}};
    auto [finalized, isax_max] = internal.finalize(isax_word_settings);

    REQUIRE(finalized->get_split_ind() == split_ind);
    REQUIRE(!finalized->is_leaf());
    REQUIRE(isax_max.size() == 2);

    REQUIRE(isax_max[0] == iSaxWord({2, 2, 3}, num_bits, 2));
    REQUIRE(isax_max[1] == iSaxWord({3, 1, 3}, num_bits, 2));
}
