#include <doctest/doctest.h>
#include <fakeit/fakeit.hpp>

#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/EnvelopeEntryMerger.hpp"

bool check_merged_entries(vec<IndexEntry<Envelope>> &expected, vec<IndexEntry<Envelope>> &actual) {
    auto compare_func = [](const IndexEntry<Envelope> &a, const IndexEntry<Envelope> &b) {
        return a.m_subs_info < b.m_subs_info;
    };
    std::sort(expected.begin(), expected.end(), compare_func);
    std::sort(actual.begin(), actual.end(), compare_func);

    return expected.size() == actual.size();
}

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

        vec<IndexEntry<Envelope>> entries {
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

        vec<IndexEntry<Envelope>> entries {
            {SubsequenceInfo(0, 1, 4), {Envelope({-0.1, -0.2, 0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.6, 0.2, -0.3}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.5, -1.0, -2.3}, {1.9, -0.1, -0.4})}},
        };
        auto expected_merged_entries = entries;

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries works for overlapping entries with same symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries {
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-1.4, -0.6, -0.15}, {-0.6, 0.7, 0.8})}},
            {SubsequenceInfo(0, 6, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, -0.1, 0.4})}},
        }, expected_merged_entries {
            {SubsequenceInfo(0, 1, 10), {Envelope({-1.4, -0.6, -0.3}, {1.9, 1.1, 1.3})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }

    SUBCASE("merge_entries skips entry with non-matching symbols") {
        SaxBasedEnvelopeEntryMerger merger(3);

        vec<IndexEntry<Envelope>> entries {
            {SubsequenceInfo(0, 1, 4), {Envelope({-1.0, -0.5, -0.3}, {0.5, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-2.4, -0.6, -1.15}, {-1.6, 0.7, -0.8})}},
            {SubsequenceInfo(0, 3, 5), {Envelope({-0.9, -0.55, -0.2}, {1.9, -0.1, 0.4})}},
        }, expected_merged_entries {
            {SubsequenceInfo(0, 1, 7), {Envelope({-1.0, -0.55, -0.3}, {1.9, 1.1, 1.3})}},
            {SubsequenceInfo(0, 4, 3), {Envelope({-2.4, -0.6, -1.15}, {-1.6, 0.7, -0.8})}},
        };

        auto merged_entries = merger.merge_entries(std::move(entries));
        check_merged_entries(expected_merged_entries, merged_entries);
    }
}