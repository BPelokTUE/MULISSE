#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Search/ResultSet.hpp"

// RRangeResultSet

RRangeResultSet::RRangeResultSet(Real r) : m_r(r) {};

SearchType RRangeResultSet::get_type() const { return R_RANGE; };

void RRangeResultSet::insert(SearchResult result) {
    if (result.distance <= m_r) m_results.push_back(result);
};

vec<SearchResult> RRangeResultSet::get_results() const { return m_results; };

Real RRangeResultSet::get_distance_lb() const { return m_r; };

void RRangeResultSet::clear() { m_results.clear(); }

Real RRangeResultSet::get_r() const { return m_r; };

// KnnResultSet

KnnResultSet::KnnResultSet(uint k) : m_k(k) {};

SearchType KnnResultSet::get_type() const { return KNN; };

void KnnResultSet::insert(SearchResult result) {
    auto it = std::lower_bound(m_results.begin(), m_results.end(), result);
    m_results.insert(it, result);
    if (m_results.size() > m_k) m_results.pop_back();
};

vec<SearchResult> KnnResultSet::get_results() const { return m_results; };

Real KnnResultSet::get_distance_lb() const { return m_results.size() < m_k ? INF : m_results[m_k - 1].distance; };

void KnnResultSet::clear() { m_results.clear(); }

uint KnnResultSet::get_k() const { return m_k; };
