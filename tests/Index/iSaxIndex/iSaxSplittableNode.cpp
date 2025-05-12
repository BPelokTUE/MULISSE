#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>

#include "Index/iSaxIndex/SplittableISaxNode.hpp"
#include "Util/RunSettings/BreakpointProperties.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

TEST_CASE("iSAX leaf finalization works with Envelope") {
    vec<SubsequenceInfo> subsequence_positions{{2, 51, 90}, {1, 26, 21}, {2, 63, 87}};
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

    SUBCASE("Finalization without merge") {
        auto finalization_result_ptr = leaf.finalize(false);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);
        auto isax_max = std::move(finalization_result->m_isax_max);

        REQUIRE(finalized->is_leaf());
        REQUIRE(finalized->get_subsequence_infos() == subsequence_positions);
        REQUIRE(isax_max.size() == 2);
        REQUIRE(isax_max[0] == iSaxWord({4, 6}, 3));
        REQUIRE(isax_max[1] == iSaxWord({2, 5}, 3));
    }

    SUBCASE("Finalization with merge") {
        auto finalization_result_ptr = leaf.finalize(true);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);
        auto isax_max = std::move(finalization_result->m_isax_max);

        vec<SubsequenceInfo> merged_subs_infos = {{1, 26, 21}, {2, 51, 99}};

        REQUIRE(finalized->is_leaf());
        REQUIRE(finalized->get_subsequence_infos() == merged_subs_infos);
        REQUIRE(isax_max.size() == 2);
        REQUIRE(isax_max[0] == iSaxWord({4, 6}, 3));
        REQUIRE(isax_max[1] == iSaxWord({2, 5}, 3));
    }
}

uptr<FinalizationResult> expected_envelope_finalization_result(FinalizedISaxNode<EnvelopeTag> *fin_child,
                                                               vec<iSaxWord> &isax_max_child) {
    auto envelope_fin_result =
        new EnvelopeFinalizationResult(uptr<FinalizedISaxNode<EnvelopeTag>>(fin_child), isax_max_child);
    return uptr<FinalizationResult>(envelope_fin_result);
}

TEST_CASE("iSAX internal finalization works with Envelope") {
    fakeit::Mock<SplittableISaxNode<Envelope>> left, right;
    fakeit::Mock<FinalizedISaxNode<EnvelopeTag>> fin_left, fin_right;

    vec<iSaxWord> isax_max_left = {iSaxWord({2, 2, 1}, 2), iSaxWord({3, 1, 2}, 2)},
                  isax_max_right = {iSaxWord({1, 1, 3}, 2), iSaxWord({3, 1, 3}, 2)};

    fakeit::When(Method(left, finalize)).Return(expected_envelope_finalization_result(&fin_left.get(), isax_max_left));
    fakeit::When(Method(right, finalize))
        .Return(expected_envelope_finalization_result(&fin_right.get(), isax_max_right));

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

    SUBCASE("Finalization without merge") {
        auto finalization_result_ptr = internal.finalize(false);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);
        auto isax_max = std::move(finalization_result->m_isax_max);

        REQUIRE(finalized->get_split_ind() == split_ind);
        REQUIRE(!finalized->is_leaf());
        REQUIRE(isax_max.size() == 2);

        REQUIRE(isax_max[0] == iSaxWord({2, 2, 3}, 2));
        REQUIRE(isax_max[1] == iSaxWord({3, 1, 3}, 2));
    }

    SUBCASE("Finalization with merge") {
        auto finalization_result_ptr = internal.finalize(false);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);
        auto isax_max = std::move(finalization_result->m_isax_max);

        REQUIRE(finalized->get_split_ind() == split_ind);
        REQUIRE(!finalized->is_leaf());
        REQUIRE(isax_max.size() == 2);

        REQUIRE(isax_max[0] == iSaxWord({2, 2, 3}, 2));
        REQUIRE(isax_max[1] == iSaxWord({3, 1, 3}, 2));
    }
}

TEST_CASE("iSAX leaf finalization works with PAA") {
    vec<SubsequenceInfo> subsequence_positions{{2, 51, 90}, {3, 26, 21}, {2, 63, 87}};
    vec<vec<Paa>> paa_values = {{{{R(-1.5), R(2.3)}}, {{R(-7.9), R(0.5)}}},
                                {{{R(-2.3), R(3.6)}}, {{R(-8.1), R(-0.5)}}},
                                {{{R(-1.9), R(1.9)}}, {{R(-10), R(-0.9)}}}};

    iSaxSplittableLeaf leaf(subsequence_positions, paa_values);

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

    SUBCASE("Finalization without merge") {
        auto finalization_result_ptr = leaf.finalize(false);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);

        REQUIRE(finalized->is_leaf());
        REQUIRE(finalized->get_subsequence_infos() == subsequence_positions);
    }

    SUBCASE("Finalization with merge") {
        auto finalization_result_ptr = leaf.finalize(true);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);

        vec<SubsequenceInfo> merged_subs_infos = {{2, 51, 99}, {3, 26, 21}};

        REQUIRE(finalized->is_leaf());
        REQUIRE(finalized->get_subsequence_infos() == merged_subs_infos);
    }
}

uptr<FinalizationResult> expected_paa_finalization_result(FinalizedISaxNode<PaaTag> *fin_child) {
    auto envelope_fin_result = new PaaFinalizationResult(uptr<FinalizedISaxNode<PaaTag>>(fin_child));
    return uptr<FinalizationResult>(envelope_fin_result);
}

TEST_CASE("iSAX internal finalization works with Envelope") {
    fakeit::Mock<SplittableISaxNode<Paa>> left, right;
    fakeit::Mock<FinalizedISaxNode<PaaTag>> fin_left, fin_right;

    fakeit::When(Method(left, finalize)).Return(expected_paa_finalization_result(&fin_left.get()));
    fakeit::When(Method(right, finalize)).Return(expected_paa_finalization_result(&fin_right.get()));

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

    SUBCASE("Finalization without merge") {
        auto finalization_result_ptr = internal.finalize(false);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);

        REQUIRE(finalized->get_split_ind() == split_ind);
        REQUIRE(!finalized->is_leaf());
    }

    SUBCASE("Finalization with merge") {
        auto finalization_result_ptr = internal.finalize(true);
        auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
        auto finalized = std::move(finalization_result->m_finalized_node);

        REQUIRE(finalized->get_split_ind() == split_ind);
        REQUIRE(!finalized->is_leaf());
    }
}
