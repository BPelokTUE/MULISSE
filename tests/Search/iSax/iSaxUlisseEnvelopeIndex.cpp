#include "doctest/doctest.h"
#include "fakeit/fakeit.hpp"

#include "Search/iSax/iSaxUlisseEnvelopeIndex.hpp"

TEST_CASE("iSaxUlisseEnvelopeIndex insert UTS envelope works") {
    fakeit::Mock<IiSaxBreakpointStrategy> breakpoint_strategy_mock;
    fakeit::Mock<IiSaxSplitStrategy> split_strategy_mock;
    std::unique_ptr<iSaxUlisseEnvelopeIndex> index;

    fakeit::Fake(Method(breakpoint_strategy_mock, get_breakpoints));
    fakeit::When(Method(breakpoint_strategy_mock, get_breakpoints)(2)).AlwaysReturn(vec<float>{0.0});
    fakeit::When(Method(breakpoint_strategy_mock, get_breakpoints)(4)).AlwaysReturn(vec<float>{-2.0, 0.0, 2.0});
    index = std::make_unique<iSaxUlisseEnvelopeIndex>(
        1, 2, std::unique_ptr<IiSaxBreakpointStrategy>(&breakpoint_strategy_mock.get()),
        std::unique_ptr<IiSaxSplitStrategy>(&split_strategy_mock.get()));

    fakeit::Fake(Method(split_strategy_mock, get_split_ind));
    fakeit::When(Method(split_strategy_mock, get_split_ind)).Return(0, 1);

    SUBCASE("inserting first envelope works") {
        index->insert({{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13);
        auto sax_min = SaxWord({0, 1, 0}, 1);
        const iSaxNode *node = index->get_first_layer_node(sax_min);
        REQUIRE(node != nullptr);

        REQUIRE(node->is_leaf());
        REQUIRE(node->get_file_positions() == vec<FilePositionT>{13});
        REQUIRE(node->get_envelopes() == vec<UlisseEnvelope>{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}});
    }

    SUBCASE("inserting envelope with existing iSAX without splitting works") {
        index->insert({{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13);

        index->insert({{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269);
        auto sax_min = SaxWord({0, 1, 0}, 1);
        const iSaxNode *node = index->get_first_layer_node(sax_min);
        REQUIRE(node != nullptr);

        REQUIRE(node->is_leaf());
        REQUIRE(node->get_file_positions() == vec<FilePositionT>{13, 1269});
        REQUIRE(node->get_envelopes() ==
                vec<UlisseEnvelope>{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}, {{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}});
    }

    SUBCASE("inserting envelope with new iSAX works") {
        index->insert({{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13);
        index->insert({{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269);

        index->insert({{{4.2, 2.4, -5.7}, {8.8, 3.8, 5.3}}}, 352);
        auto sax_min = SaxWord({1, 1, 0}, 1);
        const iSaxNode *node = index->get_first_layer_node(sax_min);
        REQUIRE(node != nullptr);

        REQUIRE(node->is_leaf());
        REQUIRE(node->get_file_positions() == vec<FilePositionT>{352});
        REQUIRE(node->get_envelopes() == vec<UlisseEnvelope>{{{4.2, 2.4, -5.7}, {8.8, 3.8, 5.3}}});
    }

    SUBCASE("inserting envelope with existing iSAX with splitting works") {
        index->insert({{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13);
        index->insert({{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269);

        index->insert({{{-4.1, 4.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891);

        auto sax_min = SaxWord({0, 1, 0}, 1);
        const iSaxNode *node = index->get_first_layer_node(sax_min);
        REQUIRE(node != nullptr);

        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto left = children.first, right = children.second;
        REQUIRE(left != nullptr);
        REQUIRE(right != nullptr);

        REQUIRE(left->is_leaf());
        REQUIRE(left->get_file_positions() == vec<FilePositionT>{1269, 7891});
        REQUIRE(left->get_envelopes() ==
                vec<UlisseEnvelope>{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}, {{-4.1, 4.5, -1.6}, {-1.8, 6.9, -0.6}}});

        REQUIRE(right->is_leaf());
        REQUIRE(right->get_file_positions() == vec<FilePositionT>{13});
        REQUIRE(right->get_envelopes() == vec<UlisseEnvelope>{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}});
    }

    SUBCASE("inserting envelope into non-first-layer node without splitting works") {
        index->insert({{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13);
        index->insert({{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269);
        // Triggers first split
        index->insert({{{-4.1, 4.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891);
        // Insert into right leaf
        index->insert({{{-0.6, 1.3, -10.6}, {1.8, 3.1, -5.6}}}, 555);

        auto sax_min = SaxWord({0, 1, 0}, 1);
        const iSaxNode *node = index->get_first_layer_node(sax_min);
        REQUIRE(node != nullptr);

        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto right = children.second;
        REQUIRE(right != nullptr);

        REQUIRE(right->is_leaf());
        REQUIRE(right->get_file_positions() == vec<FilePositionT>{13, 555});
        REQUIRE(right->get_envelopes() ==
                vec<UlisseEnvelope>{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}, {{-0.6, 1.3, -10.6}, {1.8, 3.1, -5.6}}});
    }

    // SUBCASE("inserting envelope into non-first-layer node with splitting works") {
    //     index->insert({{{-3.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13);
    //     index->insert({{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269);
    //     // Triggers first split
    //     index->insert({{{-4.1, 1.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891);
    //     // Insert into right leaf
    //     index->insert({{{-2.6, 5.3, -10.6}, {1.8, 3.1, -5.6}}}, 555);

    //     auto sax_min = SaxWord({0, 1, 0}, 1);
    //     const iSaxNode *node = index->get_first_layer_node(sax_min);
    //     REQUIRE(node != nullptr);

    //     auto *internal = dynamic_cast<const iSaxInternalNode *>(node);
    //     REQUIRE(internal != nullptr);

    //     auto children = internal->get_children();
    //     auto left = children.first;
    //     REQUIRE(left != nullptr);

    //     auto *left_internal = dynamic_cast<const iSaxSplittableLeaf *>(left);
    //     REQUIRE(left_internal != nullptr);
    //     children = left_internal->get_children();

    //     left = children.first;
    //     REQUIRE(left != nullptr);
    //     auto right = children.second;
    //     REQUIRE(right != nullptr);

    //     auto *left_leaf = dynamic_cast<const iSaxSplittableLeaf *>(left);
    //     REQUIRE(left_leaf != nullptr);
    //     REQUIRE(left_leaf->get_file_positions() == vec<FilePositionT>{13, 7891});
    //     REQUIRE(left_leaf->get_envelopes() ==
    //             vec<UlisseEnvelope>{{{-3.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}, {{-4.1, 1.5, -1.6}, {-1.8, 6.9, -0.6}}});

    //     REQUIRE(right_leaf->get_file_positions() == vec<FilePositionT>{13, 555});
    //     REQUIRE(right_leaf->get_envelopes() ==
    //             vec<UlisseEnvelope>{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}, {{-0.6, 1.3, -10.6}, {1.8, 3.1, -5.6}}});
    // }
}
