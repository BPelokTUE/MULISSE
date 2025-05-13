#include "common.hpp"

void require_paa_entries_equal(const vec<IndexEntry<Paa>> &actual, const vec<IndexEntry<Paa>> &expected) {
    REQUIRE(actual.size() == expected.size());
    for (size_t i = 0; i < actual.size(); ++i) {
        REQUIRE(actual[i].m_subs_info == expected[i].m_subs_info);
        REQUIRE(actual[i].m_mts_summary.size() == expected[i].m_mts_summary.size());
        for (size_t j = 0; j < actual[i].m_mts_summary.size(); ++j) {
            REQUIRE(actual[i].m_mts_summary[j].size() == expected[i].m_mts_summary[j].size());
            for (size_t k = 0; k < actual[i].m_mts_summary[j].size(); ++k) {
                REQUIRE_EQ(actual[i].m_mts_summary[j].m_paa_values[k],
                           doctest::Approx(expected[i].m_mts_summary[j].m_paa_values[k]).epsilon(1e-5));
            }
        }
    }
}

void require_sorted_paa_entries_equal(vec<IndexEntry<Paa>> &actual, vec<IndexEntry<Paa>> &expected) {
    auto compare_func = [](const IndexEntry<Paa> &a, const IndexEntry<Paa> &b) {
        if (a.m_subs_info != b.m_subs_info) return a.m_subs_info < b.m_subs_info;

        for (MtsNumChannelsT i = 0; i < a.m_mts_summary.size(); ++i) {
            if (a.m_mts_summary[i].m_paa_values != b.m_mts_summary[i].m_paa_values)
                return a.m_mts_summary[i].m_paa_values < b.m_mts_summary[i].m_paa_values;
        }
        return false;
    };
    std::sort(expected.begin(), expected.end(), compare_func);
    std::sort(actual.begin(), actual.end(), compare_func);

    require_paa_entries_equal(actual, expected);
}

void require_envelope_entries_equal(vec<IndexEntry<Envelope>> &actual, vec<IndexEntry<Envelope>> &expected) {
    REQUIRE(expected.size() == actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        for (size_t j = 0; j < expected[i].m_mts_summary.size(); ++j) {
            REQUIRE(expected[i].m_subs_info == actual[i].m_subs_info);
            REQUIRE(expected[i].m_mts_summary[j].size() == actual[i].m_mts_summary[j].size());
            for (size_t k = 0; k < expected[i].m_mts_summary[j].size(); ++k) {
                REQUIRE(expected[i].m_mts_summary[j].m_lower[k] ==
                        doctest::Approx(actual[i].m_mts_summary[j].m_lower[k]).epsilon(1e-5));
                REQUIRE(expected[i].m_mts_summary[j].m_upper[k] ==
                        doctest::Approx(actual[i].m_mts_summary[j].m_upper[k]).epsilon(1e-5));
            }
        }
    }
}

void require_sorted_envelope_entries_equal(vec<IndexEntry<Envelope>> &actual, vec<IndexEntry<Envelope>> &expected) {
    auto compare_func = [](const IndexEntry<Envelope> &a, const IndexEntry<Envelope> &b) {
        if (a.m_subs_info != b.m_subs_info) return a.m_subs_info < b.m_subs_info;

        for (MtsNumChannelsT i = 0; i < a.m_mts_summary.size(); ++i) {
            if (a.m_mts_summary[i].m_lower != b.m_mts_summary[i].m_lower)
                return a.m_mts_summary[i].m_lower < b.m_mts_summary[i].m_lower;
            if (a.m_mts_summary[i].m_upper != b.m_mts_summary[i].m_upper)
                return a.m_mts_summary[i].m_upper < b.m_mts_summary[i].m_upper;
        }
        return false;
    };
    std::sort(expected.begin(), expected.end(), compare_func);
    std::sort(actual.begin(), actual.end(), compare_func);

    require_envelope_entries_equal(actual, expected);
}
