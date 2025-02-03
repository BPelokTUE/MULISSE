#include "Search/ResultSet.hpp"

// RRangeResultSet

void RRangeResultSet::insert(SearchResult result) {
    assert(result.distance <= m_r);

    m_results.push_back(result);
};

vec<SearchResult> RRangeResultSet::get_results() const { return m_results; };

DistanceT RRangeResultSet::get_distance_lb() const { return m_r; };

// KnnResultSet

void KnnResultSet::insert(SearchResult result) {
    auto it = std::lower_bound(m_results.begin(), m_results.end(), result);
    m_results.insert(it, result);
    if (m_results.size() > m_k) m_results.pop_back();
};

vec<SearchResult> KnnResultSet::get_results() const { return m_results; };

DistanceT KnnResultSet::get_distance_lb() const { return m_results.empty() ? 0 : m_results.back().distance; };
