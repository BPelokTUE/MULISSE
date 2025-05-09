#include <doctest/doctest.h>
#include <fakeit/fakeit.hpp>

#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/EnvelopeEntryMerger.hpp"

void check_merged_entries(vec<IndexEntry<Envelope>> &expected, vec<IndexEntry<Envelope>> &actual) {
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

// SaxBasedEnvelopeEntryMerger tests

TEST_CASE("SaxBasedEntryMerger with Envelope type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {-1.5, -0.67, -0.4, 0, 0.4, 0.67, 1.5};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SUBCASE("merge_entries works for no overlapping entries") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 0, 5), {Envelope({-0.1, -0.2, 0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 7, 3), {Envelope({-1.6, 0.2, -0.3}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 11, 5), {Envelope({-0.5, -1.0, -2.3}, {1.9, -0.1, -0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with different symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-0.1, -0.2, 0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.6, 0.2, -0.3}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.5, -1.0, -2.3}, {1.9, -0.1, -0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same lower symbols but different upper symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.4, -0.6, -0.15}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, -0.1, 0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same upper symbols but different lower symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-0.1, -0.2, 0.3}, {1.55, 0.3, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.6, 0.2, -0.3}, {1.6, 0.25, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.5, -1.0, -2.3}, {1.9, 0.1, 1.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for non-overlapping entries with same symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 2, 4), {Envelope({-1.0, -0.5, -0.3}, {1.55, 0.3, 1.3})}},
            {SubsequenceInfo(0, 7, 3), {Envelope({-1.4, -0.6, -0.15}, {1.6, 0.25, 0.8})}},
            {SubsequenceInfo(0, 20, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, 0.1, 1.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {1.55, 0.3, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.4, -0.6, -0.15}, {1.6, 0.25, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, 0.1, 1.4})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 10), {Envelope({-1.4, -0.6, -0.3}, {1.9, 0.3, 1.4})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {1.55, 0.3, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-2.4, -0.6, -1.15}, {-1.6, 0.7, -0.8})}},
            {SubsequenceInfo(0, 3, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, 0.1, 1.4})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 7), {Envelope({-1.0, -0.55, -0.3}, {1.9, 0.3, 1.4})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-2.4, -0.6, -1.15}, {-1.6, 0.7, -0.8})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works when num_bits is less than alphabet_num_bits") {
        SaxBasedEnvelopeEntryMerger merger(2);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.7, -0.2, -1.3}, {1.5, 0.6, 0.3})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({-1.5, -0.25, -0.5}, {0.35, 0.6, 1.8})}},
            {SubsequenceInfo(0, 4, 6), {Envelope({-1.55, -0.3, -0.9}, {1.9, 0.6, 0.5})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries = {
            {SubsequenceInfo(0, 1, 9), {Envelope({-1.7, -0.3, -1.3}, {1.9, 0.6, 0.5})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({-1.5, -0.25, -0.5}, {0.35, 0.6, 1.8})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }
}

// LowerSaxBasedEntryMerger tests

TEST_CASE("LowerSaxBasedEntryMerger with Envelope type") {
    fakeit::Mock<RunSettings> run_settings_mock;

    vec<Real> breakpoints_mock = {-1.5, -0.67, -0.4, 0, 0.4, 0.67, 1.5};
    BreakpointProperties breakpoint_props_mock;
    breakpoint_props_mock.m_breakpoint_num_bits = 3;

    fakeit::When(Method(run_settings_mock, get_breakpoints)).AlwaysReturn(breakpoints_mock);
    fakeit::When(Method(run_settings_mock, get_breakpoint_props)).AlwaysReturn(breakpoint_props_mock);

#ifdef ENABLE_TEST_CODE
    // Pass empty deleter function, because fakeit manages the lifetime of the mock
    RunSettings::set_instance(sptr<RunSettings>(&run_settings_mock.get(), [](RunSettings *) {}));
#endif

    SUBCASE("merge_entries works for no overlapping entries") {
        LowerSaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 0, 5), {Envelope({-0.1, -0.2, 0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 7, 3), {Envelope({-1.6, 0.2, -0.3}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 11, 5), {Envelope({-0.5, -1.0, -2.3}, {1.9, -0.1, -0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with different symbols") {
        LowerSaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-0.1, -0.2, 0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.6, 0.2, -0.3}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.5, -1.0, -2.3}, {1.9, -0.1, -0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for non-overlapping entries with same symbols") {
        LowerSaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 6, 3), {Envelope({-1.4, -0.6, -0.15}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 11, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, -0.1, 0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        LowerSaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.4, -0.6, -0.15}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, -0.1, 0.4})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 10), {Envelope({-1.4, -0.6, -0.3}, {1.9, 1.1, 1.3})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        LowerSaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-2.4, -0.6, -1.15}, {-1.6, 0.7, -0.8})}},
            {SubsequenceInfo(0, 3, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, -0.1, 0.4})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries{
            {SubsequenceInfo(0, 1, 7), {Envelope({-1.0, -0.55, -0.3}, {1.9, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-2.4, -0.6, -1.15}, {-1.6, 0.7, -0.8})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works when num_bits is less than alphabet_num_bits") {
        LowerSaxBasedEnvelopeEntryMerger merger(2);

        vec<IndexEntry<Envelope>> entries{
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.7, -0.2, -1.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({-1.5, -0.25, -0.5}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 4, 6), {Envelope({-1.55, -0.3, -0.9}, {1.9, -0.1, -0.4})}},
        };
        vec<IndexEntry<Envelope>> expected_merged_entries = {
            {SubsequenceInfo(0, 1, 9), {Envelope({-1.7, -0.3, -1.3}, {1.9, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 5), {Envelope({-1.5, -0.25, -0.5}, {-0.6, 0.7, 0.8})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }
}
