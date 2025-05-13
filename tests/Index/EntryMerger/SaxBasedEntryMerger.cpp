#include "Index/EntryMerger/SaxBasedEntryMerger.hpp"

#include <doctest/doctest.h>

#include <fakeit/fakeit.hpp>

#include "common.hpp"

TEST_CASE("SaxBasedEntryMerger with Paa type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), R(0.0), R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;
    breakpoint_props_mock.m_breakpoints = breakpoints_mock;

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
        require_sorted_paa_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_paa_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_paa_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_paa_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_paa_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_paa_entries_equal(expected_merged_entries, merged_entries);
    }
}

// SaxBasedEntryMerger<Envelope> tests

TEST_CASE("SaxBasedEntryMerger with Envelope type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {R(-1.5), R(-0.67), R(-0.4), R(0.0), R(0.4), R(0.67), R(1.5)};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;
    breakpoint_props_mock.m_breakpoints = breakpoints_mock;

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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
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
        require_sorted_envelope_entries_equal(expected_merged_entries, merged_entries);
    }
}
