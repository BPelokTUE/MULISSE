#include "doctest/doctest.h"
#include "fakeit/fakeit.hpp"

#include "Search/iSax/iSaxEnvelopeIndex.hpp"
#include "Util/RunSettings.hpp"

// TODO: Figure out how to mock `RunSettings` and fix the tests

// void check_envelope_equality(const vec<vec<Envelope>> &expected, const vec<vec<Envelope>> &actual) {
//     REQUIRE(expected.size() == actual.size());
//     for (size_t i = 0; i < expected.size(); i++) {
//         REQUIRE(expected[i].size() == actual[i].size());
//         for (size_t j = 0; j < expected[i].size(); j++) {
//             for (size_t k = 0; k < expected[i][j].lower.size(); k++) {
//                 REQUIRE(expected[i][j].lower[k] == doctest::Approx(actual[i][j].lower[k]));
//                 REQUIRE(expected[i][j].upper[k] == doctest::Approx(actual[i][j].upper[k]));
//             }
//         }
//     }
// }

// TEST_CASE("iSaxEnvelopeIndex insert UTS envelope works") {
//     fakeit::Mock<RunSettings> run_settings_mock;
//     fakeit::Mock<IiSaxSplitStrategy> split_strategy_mock;
//     std::unique_ptr<iSaxEnvelopeIndex> index;

//     fakeit::Fake(Method(run_settings_mock, get_instance));

//     fakeit::When(Method(breakpoint_strategy_mock, get_breakpoints)(2)).AlwaysReturn(vec<float>{0.0});
//     fakeit::When(Method(breakpoint_strategy_mock, get_breakpoints)(4)).AlwaysReturn(vec<float>{-2.0, 0.0, 2.0});
//     SeriesISaxProperties series_isax_prop = {3, 100, 11, 1, 3};  // ..., 1 channel, 3 segments
//     index = std::make_unique<iSaxEnvelopeIndex>(
//         series_isax_prop, 1, 2, std::unique_ptr<IiSaxBreakpointStrategy>(&breakpoint_strategy_mock.get()),
//         std::unique_ptr<IiSaxSplitStrategy>(&split_strategy_mock.get()), 2);

//     fakeit::Fake(Method(split_strategy_mock, get_split_ind));
//     std::pair<SaxSegIndT, MtsNumChannelsT> split1{0, 0}, split2{1, 0}, split3{0, 0};
//     fakeit::When(Method(split_strategy_mock, get_split_ind)).Return(split1, split2, split3);

//     SUBCASE("inserting first envelope works") {
//         EnvelopeEntry entry = {{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);
//         auto isax_min = iSaxWord({0, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);

//         REQUIRE(node->is_leaf());
//         REQUIRE(node->get_file_positions() == vec<FilePositionT>{13});
//         check_envelope_equality(node->get_envelopes(), vec<vec<Envelope>>{{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}});
//     }

//     SUBCASE("inserting envelope with existing iSAX without splitting works") {
//         EnvelopeEntry entry = {{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);

//         entry = {{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269};
//         index->insert(entry);
//         auto isax_min = iSaxWord({0, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);

//         REQUIRE(node->is_leaf());
//         REQUIRE(node->get_file_positions() == vec<FilePositionT>{13, 1269});
//         check_envelope_equality(node->get_envelopes(), vec<vec<Envelope>>{{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}},
//                                                                           {{{-9.1, 10.3, -0.3}, {-3.8, 11.9,
//                                                                           0.6}}}});
//     }

//     SUBCASE("inserting envelope with new iSAX works") {
//         EnvelopeEntry entry = {{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);
//         entry = {{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269};
//         index->insert(entry);

//         entry = {{{{4.2, 2.4, -5.7}, {8.8, 3.8, 5.3}}}, 352};
//         index->insert(entry);
//         auto isax_min = iSaxWord({1, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);

//         REQUIRE(node->is_leaf());
//         REQUIRE(node->get_file_positions() == vec<FilePositionT>{352});
//         check_envelope_equality(node->get_envelopes(), vec<vec<Envelope>>{{{{4.2, 2.4, -5.7}, {8.8, 3.8, 5.3}}}});
//     }

//     SUBCASE("inserting envelope with existing iSAX with splitting works") {
//         EnvelopeEntry entry = {{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);
//         entry = {{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269};
//         index->insert(entry);

//         entry = {{{{-4.1, 4.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891};
//         index->insert(entry);

//         auto isax_min = iSaxWord({0, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);

//         REQUIRE(!(node->is_leaf()));

//         auto children = node->get_children();
//         auto left = children.first, right = children.second;
//         REQUIRE(left != nullptr);
//         REQUIRE(right != nullptr);

//         REQUIRE(left->is_leaf());
//         REQUIRE(left->get_file_positions() == vec<FilePositionT>{1269, 7891});
//         check_envelope_equality(left->get_envelopes(), vec<vec<Envelope>>{{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}},
//                                                                           {{{-4.1, 4.5, -1.6}, {-1.8, 6.9, -0.6}}}});

//         REQUIRE(right->is_leaf());
//         REQUIRE(right->get_file_positions() == vec<FilePositionT>{13});
//         check_envelope_equality(right->get_envelopes(), vec<vec<Envelope>>{{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}});
//     }

//     SUBCASE("inserting envelope into non-first-layer node without splitting works") {
//         EnvelopeEntry entry = {{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);
//         entry = {{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269};
//         index->insert(entry);
//         // Triggers first split
//         entry = {{{{-4.1, 4.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891};
//         index->insert(entry);
//         // Insert into right leaf
//         entry = {{{{-0.6, 1.3, -10.6}, {1.8, 3.1, -5.6}}}, 555};
//         index->insert(entry);

//         auto isax_min = iSaxWord({0, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);

//         REQUIRE(!(node->is_leaf()));

//         auto children = node->get_children();
//         auto right = children.second;
//         REQUIRE(right != nullptr);

//         REQUIRE(right->is_leaf());
//         REQUIRE(right->get_file_positions() == vec<FilePositionT>{13, 555});
//         check_envelope_equality(right->get_envelopes(), vec<vec<Envelope>>{{{{-1.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}},
//                                                                            {{{-0.6, 1.3, -10.6}, {1.8, 3.1,
//                                                                            -5.6}}}});
//     }

//     SUBCASE("inserting envelope into non-first-layer node with one extra split works") {
//         EnvelopeEntry entry = {{{{-3.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);
//         entry = {{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269};
//         index->insert(entry);
//         // Triggers first split
//         entry = {{{{-1.6, 5.3, -10.6}, {1.8, 3.1, -5.6}}}, 555};
//         index->insert(entry);

//         auto isax_min = iSaxWord({0, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);
//         REQUIRE(!(node->is_leaf()));

//         auto children = node->get_children();
//         auto left = children.first, right = children.second;
//         REQUIRE(left != nullptr);
//         REQUIRE(left->is_leaf());

//         REQUIRE(right != nullptr);
//         REQUIRE(right->is_leaf());
//         REQUIRE(right->get_file_positions() == vec<FilePositionT>{555});
//         check_envelope_equality(right->get_envelopes(), vec<vec<Envelope>>{{{{-1.6, 5.3, -10.6}, {1.8, 3.1,
//         -5.6}}}});

//         // Trigger second split
//         entry = {{{{-4.1, 1.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891};
//         index->insert(entry);

//         node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);
//         REQUIRE(!(node->is_leaf()));

//         children = node->get_children();
//         left = children.first;
//         REQUIRE(left != nullptr);

//         REQUIRE(!(left->is_leaf()));
//         children = left->get_children();

//         left = children.first;
//         REQUIRE(left != nullptr);
//         right = children.second;
//         REQUIRE(right != nullptr);

//         REQUIRE(left->is_leaf());
//         REQUIRE(left != nullptr);
//         REQUIRE(left->get_file_positions() == vec<FilePositionT>{13, 7891});
//         check_envelope_equality(left->get_envelopes(), vec<vec<Envelope>>{{{{-3.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}},
//                                                                           {{{-4.1, 1.5, -1.6}, {-1.8, 6.9, -0.6}}}});

//         REQUIRE(right->is_leaf());
//         REQUIRE(right != nullptr);
//         REQUIRE(right->get_file_positions() == vec<FilePositionT>{1269});
//         check_envelope_equality(right->get_envelopes(), vec<vec<Envelope>>{{{{-9.1, 10.3, -0.3}, {-3.8, 11.9,
//         0.6}}}});
//     }

//     SUBCASE("inserting envelope successfully triggers two splits") {
//         EnvelopeEntry entry = {{{{-3.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}}, 13};
//         index->insert(entry);
//         entry = {{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}}, 1269};
//         index->insert(entry);
//         // Triggers two splits
//         entry = {{{{-4.1, 1.5, -1.6}, {-1.8, 6.9, -0.6}}}, 7891};
//         index->insert(entry);
//         // Insert into left->right leaf
//         entry = {{{{-2.6, 5.3, -10.6}, {1.8, 3.1, -5.6}}}, 555};
//         index->insert(entry);

//         auto isax_min = iSaxWord({0, 1, 0}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);
//         REQUIRE(!(node->is_leaf()));

//         auto children = node->get_children();
//         auto left = children.first;
//         REQUIRE(left != nullptr);

//         REQUIRE(!(left->is_leaf()));
//         children = left->get_children();

//         left = children.first;
//         REQUIRE(left != nullptr);
//         auto right = children.second;
//         REQUIRE(right != nullptr);

//         REQUIRE(left->is_leaf());
//         REQUIRE(left != nullptr);
//         REQUIRE(left->get_file_positions() == vec<FilePositionT>{13, 7891});
//         check_envelope_equality(left->get_envelopes(), vec<vec<Envelope>>{{{{-3.1, 0.1, -3.9}, {1.3, 2.3, 0.8}}},
//                                                                           {{{-4.1, 1.5, -1.6}, {-1.8, 6.9, -0.6}}}});

//         REQUIRE(right->is_leaf());
//         REQUIRE(right != nullptr);
//         REQUIRE(right->get_file_positions() == vec<FilePositionT>{1269, 555});
//         check_envelope_equality(right->get_envelopes(), vec<vec<Envelope>>{{{{-9.1, 10.3, -0.3}, {-3.8, 11.9, 0.6}}},
//                                                                            {{{-2.6, 5.3, -10.6}, {1.8, 3.1,
//                                                                            -5.6}}}});
//     }

//     SUBCASE("inserting the same envelope multiple times triggers splits until max resolution is reached") {
//         vec<Envelope> envelope = {{{0.5, 3.1, 2.8}, {1.8, 4.3, 6.8}}};
//         EnvelopeEntry entry = {envelope, 100};
//         index->insert(entry);
//         entry = {envelope, 200};
//         index->insert(entry);
//         entry = {envelope, 300};
//         index->insert(entry);

//         auto isax_min = iSaxWord({1, 1, 1}, 1);
//         const iSaxSplittableNode *node = index->get_first_layer_node({isax_min});
//         REQUIRE(node != nullptr);
//         REQUIRE(!(node->is_leaf()));

//         node = node->get_children().first;
//         REQUIRE(node != nullptr);
//         REQUIRE(!(node->is_leaf()));

//         node = node->get_children().second;
//         REQUIRE(node != nullptr);
//         REQUIRE(node->is_leaf());

//         REQUIRE(node->get_file_positions() == vec<FilePositionT>{100, 200, 300});
//         check_envelope_equality(node->get_envelopes(), vec<vec<Envelope>>{envelope, envelope, envelope});
//     }
// }

// TEST_CASE("iSaxEnvelopeIndex insert MTS envelope works") {
//     fakeit::Mock<IiSaxBreakpointStrategy> breakpoint_strategy_mock;
//     fakeit::Mock<IiSaxSplitStrategy> split_strategy_mock;
//     std::unique_ptr<iSaxEnvelopeIndex> index;

//     fakeit::Fake(Method(breakpoint_strategy_mock, get_breakpoints));
//     fakeit::When(Method(breakpoint_strategy_mock, get_breakpoints)(2)).AlwaysReturn(vec<float>{0.0});
//     fakeit::When(Method(breakpoint_strategy_mock, get_breakpoints)(4)).AlwaysReturn(vec<float>{-2.0, 0.0, 2.0});
//     SeriesISaxProperties series_isax_prop = {3, 100, 11, 3, 2};  // ..., 3 channels, 2 segments
//     index = std::make_unique<iSaxEnvelopeIndex>(
//         series_isax_prop, 1, 2, std::unique_ptr<IiSaxBreakpointStrategy>(&breakpoint_strategy_mock.get()),
//         std::unique_ptr<IiSaxSplitStrategy>(&split_strategy_mock.get()), 2);

//     fakeit::Fake(Method(split_strategy_mock, get_split_ind));
//     std::pair<SaxSegIndT, MtsNumChannelsT> split1{0, 1}, split2{1, 2};
//     fakeit::When(Method(split_strategy_mock, get_split_ind)).Return(split1, split2);

//     SUBCASE("inserting one envelope works") {
//         EnvelopeEntry entry = {{{{0.2, -5.5}, {1.1, -3.1}}, {{-1.9, 2.7}, {-0.6, 3.8}}, {{1.9, 2.7}, {3.1, 5.7}}},
//         64}; index->insert(entry);

//         vec<iSaxWord> isax_mins = {iSaxWord({1, 0}, 1), iSaxWord({0, 1}, 1), iSaxWord({1, 1}, 1)};
//         const iSaxSplittableNode *node = index->get_first_layer_node(isax_mins);

//         REQUIRE(node != nullptr);
//         REQUIRE(node->is_leaf());
//         REQUIRE(node->get_file_positions() == vec<FilePositionT>{64});
//         check_envelope_equality(
//             node->get_envelopes(),
//             vec<vec<Envelope>>{{{{0.2, -5.5}, {1.1, -3.1}}, {{-1.9, 2.7}, {-0.6, 3.8}}, {{1.9, 2.7}, {3.1, 5.7}}}});
//     }

//     SUBCASE("inserting multiple envelopes works") {
//         EnvelopeEntry entry = {{{{0.2, -5.5}, {1.1, -3.1}}, {{-1.9, 2.7}, {-0.6, 3.8}}, {{1.9, 2.7}, {3.1, 5.7}}},
//         64}; index->insert(entry); entry = {{{{0.1, -0.5}, {0.8, 1.3}}, {{-1.3, 1.7}, {0.6, 2.3}}, {{1.3, 1.7},
//         {2.3, 3.7}}}, 128}; index->insert(entry); entry = {{{{0.5, -0.3}, {0.6, 1.1}}, {{-1.1, 1.3}, {0.3, 1.7}},
//         {{1.1, 1.3}, {2.1, 3.3}}}, 256}; index->insert(entry);

//         vec<iSaxWord> isax_mins = {iSaxWord({1, 0}, 1), iSaxWord({0, 1}, 1), iSaxWord({1, 1}, 1)};
//         const iSaxSplittableNode *node = index->get_first_layer_node(isax_mins);

//         REQUIRE(node != nullptr);
//         REQUIRE(!(node->is_leaf()));

//         auto children = node->get_children();
//         auto left = children.first, right = children.second;
//         REQUIRE(left != nullptr);
//         REQUIRE(right != nullptr);

//         REQUIRE(left->is_leaf());
//         REQUIRE(left->get_file_positions() == vec<FilePositionT>{});
//         check_envelope_equality(left->get_envelopes(), vec<vec<Envelope>>{});

//         REQUIRE(!(right->is_leaf()));
//         children = right->get_children();
//         left = children.first;
//         right = children.second;

//         REQUIRE(left != nullptr);
//         REQUIRE(left->is_leaf());
//         REQUIRE(left->get_file_positions() == vec<FilePositionT>{128, 256});
//         check_envelope_equality(
//             left->get_envelopes(),
//             vec<vec<Envelope>>{{{{0.1, -0.5}, {0.8, 1.3}}, {{-1.3, 1.7}, {0.6, 2.3}}, {{1.3, 1.7}, {2.3, 3.7}}},
//                                {{{0.5, -0.3}, {0.6, 1.1}}, {{-1.1, 1.3}, {0.3, 1.7}}, {{1.1, 1.3}, {2.1, 3.3}}}});

//         REQUIRE(right != nullptr);
//         REQUIRE(right->is_leaf());
//         REQUIRE(right->get_file_positions() == vec<FilePositionT>{64});
//         check_envelope_equality(
//             right->get_envelopes(),
//             vec<vec<Envelope>>{{{{0.2, -5.5}, {1.1, -3.1}}, {{-1.9, 2.7}, {-0.6, 3.8}}, {{1.9, 2.7}, {3.1, 5.7}}}});
//     }
// }
