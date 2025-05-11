#include <doctest/doctest.h>
#include <fakeit/fakeit.hpp>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Paa.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/EntryMerger.hpp"

// SaxBasedEntryMerger<Paa> tests

void check_merged_envelope_entries(vec<IndexEntry<Paa>> &expected, vec<IndexEntry<Paa>> &actual) {
    auto compare_func = [](const IndexEntry<Paa> &a, const IndexEntry<Paa> &b) {
        return a.m_subs_info < b.m_subs_info;
    };
    std::sort(expected.begin(), expected.end(), compare_func);
    std::sort(actual.begin(), actual.end(), compare_func);

    REQUIRE(expected.size() == actual.size());

    for (size_t i = 0; i < expected.size(); ++i) {
        for (size_t j = 0; j < expected[i].m_mts_summary.size(); ++j) {
            REQUIRE(expected[i].m_subs_info == actual[i].m_subs_info);
            REQUIRE(expected[i].m_mts_summary[j].size() == actual[i].m_mts_summary[j].size());
            for (size_t k = 0; k < expected[i].m_mts_summary[j].size(); ++k) {
                REQUIRE(expected[i].m_mts_summary[j].m_paa_values[k] ==
                        doctest::Approx(actual[i].m_mts_summary[j].m_paa_values[k]));
            }
        }
    }
}

TEST_CASE("SaxBasedEntryMerger with Paa type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), R(0.0), R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SUBCASE("merge_entries works for no overlapping entries") {
        SaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 0, 5), {Paa({R(-0.1), R(-0.2), R(0.3)})}},
            {SubsequenceInfo(0, 7, 3), {Paa({R(-1.6), R(0.2), R(-0.3)})}},
            {SubsequenceInfo(0, 11, 5), {Paa({R(-0.5), R(-1.0), R(-2.3)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with different symbols") {
        SaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 3), {Paa({R(-0.1), R(-0.2), R(0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-1.6), R(0.2), R(-0.3)})}},
            {SubsequenceInfo(0, 6, 5), {Paa({R(-0.5), R(-1.0), R(-2.3)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for non-overlapping entries with same symbols") {
        SaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 2, 4), {Paa({R(-1.0), R(-0.5), R(-0.3)})}},
            {SubsequenceInfo(0, 7, 3), {Paa({R(-1.4), R(-0.6), R(-0.15)})}},
            {SubsequenceInfo(0, 20, 5), {Paa({R(-0.9), R(-0.55), R(-0.2)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        SaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 3), {Paa({R(-1.0), R(-0.5), R(-0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-1.4), R(-0.6), R(-0.15)})}},
            {SubsequenceInfo(0, 6, 5), {Paa({R(-0.9), R(-0.55), R(-0.2)})}},
        };
        vec<IndexEntry<Paa>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 10), {Paa({R(-1.4), R(-0.6), R(-0.3)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        SaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 4), {Paa({R(-1.0), R(-0.5), R(-0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-2.4), R(-0.6), R(-1.15)})}},
            {SubsequenceInfo(0, 3, 5), {Paa({R(-0.9), R(-0.55), R(-0.2)})}},
        };
        vec<IndexEntry<Paa>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 7), {Paa({R(-1.0), R(-0.55), R(-0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-2.4), R(-0.6), R(-1.15)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works when num_bits is less than alphabet_num_bits") {
        SaxBasedEntryMerger<Paa> merger(2);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 4), {Paa({R(-1.7), R(-0.2), R(-1.3)})}},
            {SubsequenceInfo(0, 4, 5), {Paa({R(-1.5), R(-0.25), R(-0.5)})}},
            {SubsequenceInfo(0, 4, 6), {Paa({R(-1.55), R(-0.3), R(-0.9)})}},
        };
        vec<IndexEntry<Paa>> expected_merged_entries = {
            {SubsequenceInfo(0, 1, 9), {Paa({R(-1.7), R(-0.3), R(-1.3)})}},
            {SubsequenceInfo(0, 4, 5), {Paa({R(-1.5), R(-0.25), R(-0.5)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }
}

// LowerSaxBasedEntryMerger<Paa> tests

TEST_CASE("LowerSaxBasedEntryMerger with Paa type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), 0, R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SUBCASE("merge_entries works for no overlapping entries") {
        LowerSaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 0, 5), {Paa({R(-0.1), R(-0.2), R(0.3)})}},
            {SubsequenceInfo(0, 7, 3), {Paa({R(-1.6), R(0.2), R(-0.3)})}},
            {SubsequenceInfo(0, 11, 5), {Paa({R(-0.5), R(-1.0), R(-2.3)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with different symbols") {
        LowerSaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 3), {Paa({R(-0.1), R(-0.2), R(0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-1.6), R(0.2), R(-0.3)})}},
            {SubsequenceInfo(0, 6, 5), {Paa({R(-0.5), R(-1.0), R(-2.3)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for non-overlapping entries with same symbols") {
        LowerSaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 3), {Paa({R(-1.0), R(-0.5), R(-0.3)})}},
            {SubsequenceInfo(0, 6, 3), {Paa({R(-1.4), R(-0.6), R(-0.15)})}},
            {SubsequenceInfo(0, 11, 5), {Paa({R(-0.9), R(-0.55), R(-0.2)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        LowerSaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 3), {Paa({R(-1.0), R(-0.5), R(-0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-1.4), R(-0.6), R(-0.15)})}},
            {SubsequenceInfo(0, 6, 5), {Paa({R(-0.9), R(-0.55), R(-0.2)})}},
        };
        vec<IndexEntry<Paa>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 10), {Paa({R(-1.4), R(-0.6), R(-0.3)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        LowerSaxBasedEntryMerger<Paa> merger(3);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 3), {Paa({R(-1.0), R(-0.5), R(-0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-2.4), R(-0.6), R(-1.15)})}},
            {SubsequenceInfo(0, 3, 5), {Paa({R(-0.9), R(-0.55), R(-0.2)})}},
        };
        vec<IndexEntry<Paa>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 7), {Paa({R(-1.0), R(-0.55), R(-0.3)})}},
            {SubsequenceInfo(0, 4, 3), {Paa({R(-2.4), R(-0.6), R(-1.15)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works when num_bits is less than alphabet_num_bits") {
        LowerSaxBasedEntryMerger<Paa> merger(2);

        vec<IndexEntry<Paa>> entries{
            {SubsequenceInfo(0, 1, 4), {Paa({R(-1.7), R(-0.2), R(-1.3)})}},
            {SubsequenceInfo(0, 4, 5), {Paa({R(-1.5), R(-0.25), R(-0.5)})}},
            {SubsequenceInfo(0, 4, 6), {Paa({R(-1.55), R(-0.3), R(-0.9)})}},
        };
        vec<IndexEntry<Paa>> expected_merged_entries = {
            {SubsequenceInfo(0, 1, 9), {Paa({R(-1.7), R(-0.3), R(-1.3)})}},
            {SubsequenceInfo(0, 4, 5), {Paa({R(-1.5), R(-0.25), R(-0.5)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }
}


// SaxBasedEntryMerger<Envelope> tests

void check_merged_envelope_entries(vec<IndexEntry<Envelope>> &expected, vec<IndexEntry<Envelope>> &actual) {
    auto compare_func = [](const IndexEntry<Envelope> &a, const IndexEntry<Envelope> &b) {
        return a.m_subs_info < b.m_subs_info;
    };
    std::sort(expected.begin(), expected.end(), compare_func);
    std::sort(actual.begin(), actual.end(), compare_func);

    REQUIRE(expected.size() == actual.size());

    for (size_t i = 0; i < expected.size(); ++i) {
        for (size_t j = 0; j < expected[i].m_mts_summary.size(); ++j) {
            REQUIRE(expected[i].m_subs_info == actual[i].m_subs_info);
            REQUIRE(expected[i].m_mts_summary[j].size() == actual[i].m_mts_summary[j].size());
            for (size_t k = 0; k < expected[i].m_mts_summary[j].size(); ++k) {
                REQUIRE(expected[i].m_mts_summary[j].m_lower[k] ==
                        doctest::Approx(actual[i].m_mts_summary[j].m_lower[k]));
                REQUIRE(expected[i].m_mts_summary[j].m_upper[k] ==
                        doctest::Approx(actual[i].m_mts_summary[j].m_upper[k]));
            }
        }
    }
}

TEST_CASE("SaxBasedEntryMerger with Envelope type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), R(0.0), R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SUBCASE("merge_entries works for no overlapping entries") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 0, 5), {Envelope({R(-0.1), R(-0.2), R(0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 7, 3), {Envelope({R(-1.6), R(0.2), R(-0.3)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 11, 5), {Envelope({R(-0.5), R(-1.0), R(-2.3)}, {R(1.9), R(-0.1), R(-0.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with different symbols") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-0.1), R(-0.2), R(0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-1.6), R(0.2), R(-0.3)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({R(-0.5), R(-1.0), R(-2.3)}, {R(1.9), R(-0.1), R(-0.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same lower symbols but different upper symbols") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-1.4), R(-0.6), R(-0.15)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(-0.1), R(0.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same upper symbols but different lower symbols") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-0.1), R(-0.2), R(0.3)}, {R(1.55), R(0.3), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-1.6), R(0.2), R(-0.3)}, {R(1.6), R(0.25), R(0.8)})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({R(-0.5), R(-1.0), R(-2.3)}, {R(1.9), R(0.1), R(1.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for non-overlapping entries with same symbols") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 2, 4), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(1.55), R(0.3), R(1.3)})}},
            {SubsequenceInfo(0, 7, 3), {Envelope({R(-1.4), R(-0.6), R(-0.15)}, {R(1.6), R(0.25), R(0.8)})}},
            {SubsequenceInfo(0, 20, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(0.1), R(1.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(1.55), R(0.3), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-1.4), R(-0.6), R(-0.15)}, {R(1.6), R(0.25), R(0.8)})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(0.1), R(1.4)})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 10), {Envelope({R(-1.4), R(-0.6), R(-0.3)}, {R(1.9), R(0.3), R(1.4)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        SaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(1.55), R(0.3), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-2.4), R(-0.6), R(-1.15)}, {R(-1.6), R(0.7), R(-0.8)})}},
            {SubsequenceInfo(0, 3, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(0.1), R(1.4)})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 7), {Envelope({R(-1.0), R(-0.55), R(-0.3)}, {R(1.9), R(0.3), R(1.4)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-2.4), R(-0.6), R(-1.15)}, {R(-1.6), R(0.7), R(-0.8)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works when num_bits is less than alphabet_num_bits") {
        SaxBasedEntryMerger<Envelope> merger(2);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({R(-1.7), R(-0.2), R(-1.3)}, {R(1.5), R(0.6), R(0.3)})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({R(-1.5), R(-0.25), R(-0.5)}, {R(0.35), R(0.6), R(1.8)})}},
            {SubsequenceInfo(0, 4, 6), {Envelope({R(-1.55), R(-0.3), R(-0.9)}, {R(1.9), R(0.6), R(0.5)})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries = {
            {SubsequenceInfo(0, 1, 9), {Envelope({R(-1.7), R(-0.3), R(-1.3)}, {R(1.9), R(0.6), R(0.5)})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({R(-1.5), R(-0.25), R(-0.5)}, {R(0.35), R(0.6), R(1.8)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }
}

// LowerSaxBasedEntryMerger tests

TEST_CASE("LowerSaxBasedEntryMerger with Envelope type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), 0, R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SUBCASE("merge_entries works for no overlapping entries") {
        LowerSaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 0, 5), {Envelope({R(-0.1), R(-0.2), R(0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 7, 3), {Envelope({R(-1.6), R(0.2), R(-0.3)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 11, 5), {Envelope({R(-0.5), R(-1.0), R(-2.3)}, {R(1.9), R(-0.1), R(-0.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with different symbols") {
        LowerSaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-0.1), R(-0.2), R(0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-1.6), R(0.2), R(-0.3)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({R(-0.5), R(-1.0), R(-2.3)}, {R(1.9), R(-0.1), R(-0.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for non-overlapping entries with same symbols") {
        LowerSaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 6, 3), {Envelope({R(-1.4), R(-0.6), R(-0.15)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 11, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(-0.1), R(0.4)})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        LowerSaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-1.4), R(-0.6), R(-0.15)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(-0.1), R(0.4)})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 10), {Envelope({R(-1.4), R(-0.6), R(-0.3)}, {R(1.9), R(1.1), R(1.3)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        LowerSaxBasedEntryMerger<Envelope> merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 3), {Envelope({R(-1.0), R(-0.5), R(-0.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-2.4), R(-0.6), R(-1.15)}, {R(-1.6), R(0.7), R(-0.8)})}},
            {SubsequenceInfo(0, 3, 5), {Envelope({R(-0.9), R(-0.55), R(-0.2)}, {R(1.9), R(-0.1), R(0.4)})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 7), {Envelope({R(-1.0), R(-0.55), R(-0.3)}, {R(1.9), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({R(-2.4), R(-0.6), R(-1.15)}, {R(-1.6), R(0.7), R(-0.8)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works when num_bits is less than alphabet_num_bits") {
        LowerSaxBasedEntryMerger<Envelope> merger(2);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({R(-1.7), R(-0.2), R(-1.3)}, {R(0.5), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({R(-1.5), R(-0.25), R(-0.5)}, {R(-0.6), R(0.7), R(0.8)})}},
            {SubsequenceInfo(0, 4, 6), {Envelope({R(-1.55), R(-0.3), R(-0.9)}, {R(1.9), R(-0.1), R(-0.4)})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries = {
            {SubsequenceInfo(0, 1, 9), {Envelope({R(-1.7), R(-0.3), R(-1.3)}, {R(1.9), R(1.1), R(1.3)})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({R(-1.5), R(-0.25), R(-0.5)}, {R(-0.6), R(0.7), R(0.8)})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_envelope_entries(expected_merged_entries, merged_entries);
    }
}
