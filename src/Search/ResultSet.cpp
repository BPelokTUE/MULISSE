#include "Search/ResultSet.hpp"

// RRangeResultSet

RRangeResultSet::RRangeResultSet(DistanceT r) : m_r(r) {};

void RRangeResultSet::insert(SearchResult result) {
    if (result.distance <= m_r) m_results.push_back(result);
};

vec<SearchResult> RRangeResultSet::get_results() const { return m_results; };

DistanceT RRangeResultSet::get_distance_lb() const { return m_r; };

// KnnResultSet

KnnResultSet::KnnResultSet(unsigned k) : m_k(k) {};

void KnnResultSet::insert(SearchResult result) {
    auto it = std::lower_bound(m_results.begin(), m_results.end(), result);
    m_results.insert(it, result);
    if (m_results.size() > m_k) m_results.pop_back();
};

vec<SearchResult> KnnResultSet::get_results() const { return m_results; };

DistanceT KnnResultSet::get_distance_lb() const { return m_results.empty() ? 0 : m_results.back().distance; };
