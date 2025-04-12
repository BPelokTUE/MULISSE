#include <doctest/doctest.h>
#include "fakeit/fakeit.hpp"

#include "Search/iSax/iSaxIndex.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"

void check_envelope_equality(const vec<vec<Envelope>> &expected, const vec<vec<Envelope>> &actual) {
    REQUIRE(expected.size() == actual.size());
    for (size_t i = 0; i < expected.size(); i++) {
        REQUIRE(expected[i].size() == actual[i].size());
        for (size_t j = 0; j < expected[i].size(); j++) {
            for (size_t k = 0; k < expected[i][j].m_lower.size(); k++) {
                REQUIRE(expected[i][j].m_lower[k] == doctest::Approx(actual[i][j].m_lower[k]));
                REQUIRE(expected[i][j].m_upper[k] == doctest::Approx(actual[i][j].m_upper[k]));
            }
        }
    }
}

TEST_CASE("iSaxIndex insert UTS envelope works") {
    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::Mock<IiSaxSplitStrategy<Envelope>> split_strategy_mock;

    vec<Real> breakpoints = {-2.0, 0.0, 2.0};
    SaxNumBitsT breakpoint_num_bits = 2;
    iSaxProperties isax_props = {33, 3, nullptr, breakpoints, breakpoint_num_bits};

    auto series_isax_prop = std::make_unique<SeriesISaxEnvelopeProperties>(isax_props.m_segment_len, 100, 1, 3, 11);
    SaxSplitIndex split1{0, 0}, split2{1, 0}, split3{0, 0};

    fakeit::When(Method(run_settings_mock, get_isax_props)).AlwaysReturn(isax_props);
    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints);
    fakeit::When(Method(split_strategy_mock, get_split_ind)).Return(split1, split2, split3);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    std::unique_ptr<iSaxEnvelopeIndex> index;
    index = std::make_unique<iSaxEnvelopeIndex>(std::move(series_isax_prop), 1, 2,
                                                uptr<IiSaxSplitStrategy<Envelope>>(&split_strategy_mock.get()));

    SUBCASE("inserting first envelope works") {
        IndexEntry<Envelope> entry = {{13, 1, 7}, {{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry);
        auto isax_min = iSaxWord({0, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);

        REQUIRE(node->is_leaf());
        REQUIRE(node->get_subsequence_infos() == vec<SubsequenceInfo>{{13, 1}});
        check_envelope_equality(node->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}});
    }

    SUBCASE("inserting envelope with existing iSAX without splitting works") {
        IndexEntry<Envelope> entry1 = {{13, 1, 7}, {{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{126, 9, 6}, {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}};
        index->insert(entry2);

        auto isax_min = iSaxWord({0, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);

        REQUIRE(node->is_leaf());
        REQUIRE(node->get_subsequence_infos() == vec<SubsequenceInfo>{{13, 1}, {126, 9}});
        check_envelope_equality(node->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}},
                                                   {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}});
    }

    SUBCASE("inserting envelope with new iSAX works") {
        IndexEntry<Envelope> entry1 = {{13, 1, 7}, {{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{126, 9, 6}, {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{352, 111, 6}, {{{R(4.2), R(2.4), R(-5.7)}, {R(8.8), R(3.8), R(5.3)}}}};
        index->insert(entry3);

        auto isax_min = iSaxWord({1, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);

        REQUIRE(node->is_leaf());
        REQUIRE(node->get_subsequence_infos() == vec<SubsequenceInfo>{{352, 111}});
        check_envelope_equality(node->get_summaries(),
                                vec<vec<Envelope>>{{{{R(4.2), R(2.4), R(-5.7)}, {R(8.8), R(3.8), R(5.3)}}}});
    }

    SUBCASE("inserting envelope with existing iSAX with splitting works") {
        IndexEntry<Envelope> entry1 = {{13, 1}, {{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{126, 9, 6}, {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{78, 91, 7}, {{{R(-4.1), R(4.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}};
        index->insert(entry3);

        auto isax_min = iSaxWord({0, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);

        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto left = children.first, right = children.second;
        REQUIRE(left != nullptr);
        REQUIRE(right != nullptr);

        REQUIRE(left->is_leaf());
        REQUIRE(left->get_subsequence_infos() == vec<SubsequenceInfo>{{126, 9}, {78, 91}});
        check_envelope_equality(left->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}},
                                                   {{{R(-4.1), R(4.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}});

        REQUIRE(right->is_leaf());
        REQUIRE(right->get_subsequence_infos() == vec<SubsequenceInfo>{{13, 1}});
        check_envelope_equality(right->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}});
    }

    SUBCASE("inserting envelope into non-first-layer node without splitting works") {
        IndexEntry<Envelope> entry1 = {{13, 1}, {{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{126, 9}, {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{78, 91}, {{{R(-4.1), R(4.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}};
        index->insert(entry3);
        IndexEntry<Envelope> entry4 = {{555, 555}, {{{R(-0.6), R(1.3), R(-10.6)}, {R(1.8), R(3.1), R(-5.6)}}}};
        index->insert(entry4);

        auto isax_min = iSaxWord({0, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);

        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto right = children.second;
        REQUIRE(right != nullptr);

        REQUIRE(right->is_leaf());
        REQUIRE(right->get_subsequence_infos() == vec<SubsequenceInfo>{{13, 1}, {555, 555}});
        check_envelope_equality(right->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-1.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}},
                                                   {{{R(-0.6), R(1.3), R(-10.6)}, {R(1.8), R(3.1), R(-5.6)}}}});
    }

    SUBCASE("inserting envelope into non-first-layer node with one extra split works") {
        IndexEntry<Envelope> entry1 = {{13, 1}, {{{R(-3.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{126, 9}, {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{555, 555}, {{{R(-1.6), R(5.3), R(-10.6)}, {R(1.8), R(3.1), R(-5.6)}}}};
        index->insert(entry3);

        auto isax_min = iSaxWord({0, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);
        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto left = children.first, right = children.second;
        REQUIRE(left != nullptr);
        REQUIRE(left->is_leaf());

        REQUIRE(right != nullptr);
        REQUIRE(right->is_leaf());
        REQUIRE(right->get_subsequence_infos() == vec<SubsequenceInfo>{{555, 555}});
        check_envelope_equality(right->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-1.6), R(5.3), R(-10.6)}, {R(1.8), R(3.1), R(-5.6)}}}});

        // Trigger second split
        IndexEntry<Envelope> entry4 = {{78, 91}, {{{R(-4.1), R(1.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}};
        index->insert(entry4);

        node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);
        REQUIRE(!(node->is_leaf()));

        children = node->get_children();
        left = children.first;
        REQUIRE(left != nullptr);

        REQUIRE(!(left->is_leaf()));
        children = left->get_children();

        left = children.first;
        REQUIRE(left != nullptr);
        right = children.second;
        REQUIRE(right != nullptr);

        REQUIRE(left->is_leaf());
        REQUIRE(left != nullptr);
        REQUIRE(left->get_subsequence_infos() == vec<SubsequenceInfo>{{13, 1}, {78, 91}});
        check_envelope_equality(left->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-3.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}},
                                                   {{{R(-4.1), R(1.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}});

        REQUIRE(right->is_leaf());
        REQUIRE(right != nullptr);
        REQUIRE(right->get_subsequence_infos() == vec<SubsequenceInfo>{{126, 9}});
        check_envelope_equality(right->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}});
    }

    SUBCASE("inserting envelope successfully triggers two splits") {
        IndexEntry<Envelope> entry1 = {{13, 1}, {{{R(-3.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{126, 9}, {{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}}};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{78, 91}, {{{R(-4.1), R(1.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}};
        index->insert(entry3);
        IndexEntry<Envelope> entry4 = {{555, 555}, {{{R(-2.6), R(5.3), R(-10.6)}, {R(1.8), R(3.1), R(-5.6)}}}};
        index->insert(entry4);

        auto isax_min = iSaxWord({0, 1, 0}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);
        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto left = children.first;
        REQUIRE(left != nullptr);

        REQUIRE(!(left->is_leaf()));
        children = left->get_children();

        left = children.first;
        REQUIRE(left != nullptr);
        auto right = children.second;
        REQUIRE(right != nullptr);

        REQUIRE(left->is_leaf());
        REQUIRE(left != nullptr);
        REQUIRE(left->get_subsequence_infos() == vec<SubsequenceInfo>{{13, 1}, {78, 91}});
        check_envelope_equality(left->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-3.1), R(0.1), R(-3.9)}, {R(1.3), R(2.3), R(0.8)}}},
                                                   {{{R(-4.1), R(1.5), R(-1.6)}, {R(-1.8), R(6.9), R(-0.6)}}}});

        REQUIRE(right->is_leaf());
        REQUIRE(right != nullptr);
        REQUIRE(right->get_subsequence_infos() == vec<SubsequenceInfo>{{126, 9}, {555, 555}});
        check_envelope_equality(right->get_summaries(),
                                vec<vec<Envelope>>{{{{R(-9.1), R(10.3), R(-0.3)}, {R(-3.8), R(11.9), R(0.6)}}},
                                                   {{{R(-2.6), R(5.3), R(-10.6)}, {R(1.8), R(3.1), R(-5.6)}}}});
    }

    SUBCASE("inserting the same envelope multiple times triggers splits until max resolution is reached") {
        vec<Envelope> envelope = {{{R(0.5), R(3.1), R(2.8)}, {R(1.8), R(4.3), R(6.8)}}};
        IndexEntry<Envelope> entry1 = {{100, 300, 6}, envelope};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{200, 200, 6}, envelope};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{300, 100, 6}, envelope};
        index->insert(entry3);

        auto isax_min = iSaxWord({1, 1, 1}, 1);
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node({isax_min});
        REQUIRE(node != nullptr);
        REQUIRE(!(node->is_leaf()));

        node = node->get_children().first;
        REQUIRE(node != nullptr);
        REQUIRE(!(node->is_leaf()));

        node = node->get_children().second;
        REQUIRE(node != nullptr);
        REQUIRE(node->is_leaf());

        REQUIRE(node->get_subsequence_infos() == vec<SubsequenceInfo>{{100, 300}, {200, 200}, {300, 100}});
        check_envelope_equality(node->get_summaries(), vec<vec<Envelope>>{envelope, envelope, envelope});
    }
}

TEST_CASE("iSaxIndex insert MTS envelope works") {
    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::Mock<IiSaxSplitStrategy<Envelope>> split_strategy_mock;

    vec<Real> breakpoints = {-2.0, 0.0, 2.0};
    SaxNumBitsT breakpoint_num_bits = 2;
    iSaxProperties isax_props = {33, 3, nullptr, breakpoints, breakpoint_num_bits};
    auto series_isax_prop = std::make_unique<SeriesISaxEnvelopeProperties>(isax_props.m_segment_len, 100, 3, 2, 11);
    SaxSplitIndex split1{0, 1}, split2{1, 2};

    fakeit::When(Method(run_settings_mock, get_isax_props)).AlwaysReturn(isax_props);
    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints);
    fakeit::When(Method(split_strategy_mock, get_split_ind)).Return(split1, split2);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    std::unique_ptr<iSaxEnvelopeIndex> index;
    index = std::make_unique<iSaxEnvelopeIndex>(
        std::move(series_isax_prop), 1, 2, std::unique_ptr<IiSaxSplitStrategy<Envelope>>(&split_strategy_mock.get()));

    SUBCASE("inserting one envelope works") {
        IndexEntry<Envelope> entry = {{64, 37},
                                      {{{R(0.2), R(-5.5)}, {R(1.1), R(-3.1)}},
                                       {{R(-1.9), R(2.7)}, {R(-0.6), R(3.8)}},
                                       {{R(1.9), R(2.7)}, {R(3.1), R(5.7)}}}};
        index->insert(entry);

        vec<iSaxWord> isax_mins = {iSaxWord({1, 0}, 1), iSaxWord({0, 1}, 1), iSaxWord({1, 1}, 1)};
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node(isax_mins);

        REQUIRE(node != nullptr);
        REQUIRE(node->is_leaf());
        REQUIRE(node->get_subsequence_infos() == vec<SubsequenceInfo>{{64, 37}});
        check_envelope_equality(node->get_summaries(), vec<vec<Envelope>>{{{{R(0.2), R(-5.5)}, {R(1.1), R(-3.1)}},
                                                                           {{R(-1.9), R(2.7)}, {R(-0.6), R(3.8)}},
                                                                           {{R(1.9), R(2.7)}, {R(3.1), R(5.7)}}}});
    }

    SUBCASE("inserting multiple envelopes works") {
        IndexEntry<Envelope> entry1 = {{64, 37},
                                       {{{R(0.2), R(-5.5)}, {R(1.1), R(-3.1)}},
                                        {{R(-1.9), R(2.7)}, {R(-0.6), R(3.8)}},
                                        {{R(1.9), R(2.7)}, {R(3.1), R(5.7)}}}};
        index->insert(entry1);
        IndexEntry<Envelope> entry2 = {{128, 81},
                                       {{{R(0.1), R(-0.5)}, {R(0.8), R(1.3)}},
                                        {{R(-1.3), R(1.7)}, {R(0.6), R(2.3)}},
                                        {{R(1.3), R(1.7)}, {R(2.3), R(3.7)}}}};
        index->insert(entry2);
        IndexEntry<Envelope> entry3 = {{256, 19},
                                       {{{R(0.5), R(-0.3)}, {R(0.6), R(1.1)}},
                                        {{R(-1.1), R(1.3)}, {R(0.3), R(1.7)}},
                                        {{R(1.1), R(1.3)}, {R(2.1), R(3.3)}}}};
        index->insert(entry3);

        vec<iSaxWord> isax_mins = {iSaxWord({1, 0}, 1), iSaxWord({0, 1}, 1), iSaxWord({1, 1}, 1)};
        const iSaxSplittableNode<Envelope> *node = index->get_first_layer_node(isax_mins);

        REQUIRE(node != nullptr);
        REQUIRE(!(node->is_leaf()));

        auto children = node->get_children();
        auto left = children.first, right = children.second;
        REQUIRE(left != nullptr);
        REQUIRE(right != nullptr);

        REQUIRE(left->is_leaf());
        REQUIRE(left->get_subsequence_infos() == vec<SubsequenceInfo>{});
        check_envelope_equality(left->get_summaries(), vec<vec<Envelope>>{});

        REQUIRE(!(right->is_leaf()));
        children = right->get_children();
        left = children.first;
        right = children.second;

        REQUIRE(left != nullptr);
        REQUIRE(left->is_leaf());
        REQUIRE(left->get_subsequence_infos() == vec<SubsequenceInfo>{{128, 81}, {256, 19}});
        check_envelope_equality(left->get_summaries(), vec<vec<Envelope>>{{{{R(0.1), R(-0.5)}, {R(0.8), R(1.3)}},
                                                                           {{R(-1.3), R(1.7)}, {R(0.6), R(2.3)}},
                                                                           {{R(1.3), R(1.7)}, {R(2.3), R(3.7)}}},
                                                                          {{{R(0.5), R(-0.3)}, {R(0.6), R(1.1)}},
                                                                           {{R(-1.1), R(1.3)}, {R(0.3), R(1.7)}},
                                                                           {{R(1.1), R(1.3)}, {R(2.1), R(3.3)}}}});

        REQUIRE(right != nullptr);
        REQUIRE(right->is_leaf());
        REQUIRE(right->get_subsequence_infos() == vec<SubsequenceInfo>{{64, 37}});
        check_envelope_equality(right->get_summaries(), vec<vec<Envelope>>{{{{R(0.2), R(-5.5)}, {R(1.1), R(-3.1)}},
                                                                            {{R(-1.9), R(2.7)}, {R(-0.6), R(3.8)}},
                                                                            {{R(1.9), R(2.7)}, {R(3.1), R(5.7)}}}});
    }
}
