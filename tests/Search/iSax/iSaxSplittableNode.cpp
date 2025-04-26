#include <doctest/doctest.h>
#include <fakeit/fakeit.hpp>

#include "Util/utilities.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"

TEST_CASE("iSAX leaf finalization works") {
    vec<SubsequenceInfo> subsequence_positions{{51, 100}, {26, 21}, {6, 387}};
    vec<vec<Envelope>> envelopes = {{{{R(-1.5), R(2.3)}, {R(-0.1), R(4.9)}}, {{R(-7.9), R(0.5)}, {R(-3.3), R(2.7)}}},
                                    {{{R(-2.3), R(3.6)}, {R(0.6), R(9.7)}}, {{R(-8.1), R(-0.5)}, {R(-1.5), R(6.3)}}},
                                    {{{R(-1.9), R(1.9)}, {R(1.9), R(7.1)}}, {{R(-10), R(-0.9)}, {R(-9), R(1.9)}}}};

    iSaxSplittableLeaf leaf(subsequence_positions, envelopes);

    vec<Real> breakpoints = {-7, -3, -1, 1, 4, 9, 10};
    SaxNumBitsT breakpoint_num_bits = 3;
    BreakpointProperties breakpoint_props = {breakpoint_num_bits, nullptr, breakpoints};

    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props);
    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    auto finalization_result_ptr = leaf.finalize();
    auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
    auto finalized = std::move(finalization_result->m_finalized_node);
    auto isax_max = std::move(finalization_result->m_isax_max);

    REQUIRE(finalized->is_leaf());
    REQUIRE(finalized->get_subsequence_infos() == subsequence_positions);
    REQUIRE(isax_max.size() == 2);
    REQUIRE(isax_max[0] == iSaxWord({4, 6}, 3));
    REQUIRE(isax_max[1] == iSaxWord({2, 5}, 3));
}

uptr<FinalizationResult> expected_finalization_result(iSaxFinalizedNode<EnvelopeTag> *fin_child,
                                                      vec<iSaxWord> &isax_max_child) {
    auto envelope_fin_result =
        new EnvelopeFinalizationResult(uptr<iSaxFinalizedNode<EnvelopeTag>>(fin_child), isax_max_child);
    return uptr<FinalizationResult>(envelope_fin_result);
}

TEST_CASE("iSAX internal finalization works") {
    fakeit::Mock<iSaxSplittableNode<Envelope>> left, right;
    fakeit::Mock<iSaxFinalizedNode<EnvelopeTag>> fin_left, fin_right;

    vec<iSaxWord> isax_max_left = {iSaxWord({2, 2, 1}, 2), iSaxWord({3, 1, 2}, 2)},
                  isax_max_right = {iSaxWord({1, 1, 3}, 2), iSaxWord({3, 1, 3}, 2)};

    fakeit::When(Method(left, finalize)).Return(expected_finalization_result(&fin_left.get(), isax_max_left));
    fakeit::When(Method(right, finalize)).Return(expected_finalization_result(&fin_right.get(), isax_max_right));

    vec<Real> breakpoints = {-3, 0, 3};
    SaxNumBitsT breakpoint_num_bits = 2;
    BreakpointProperties breakpoint_props = {breakpoint_num_bits, nullptr, breakpoints};

    fakeit::Mock<RunSettings> run_settings_mock;
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props);
    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SaxSplitIndex split_ind{1, 0};
    iSaxSplittableInternal internal(split_ind, &left.get(), &right.get());

    auto finalization_result_ptr = internal.finalize();
    auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
    auto finalized = std::move(finalization_result->m_finalized_node);
    auto isax_max = std::move(finalization_result->m_isax_max);

    REQUIRE(finalized->get_split_ind() == split_ind);
    REQUIRE(!finalized->is_leaf());
    REQUIRE(isax_max.size() == 2);

    REQUIRE(isax_max[0] == iSaxWord({2, 2, 3}, 2));
    REQUIRE(isax_max[1] == iSaxWord({3, 1, 3}, 2));
}
