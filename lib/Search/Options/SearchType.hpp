#ifndef SEARCH_TYPE_HPP
#define SEARCH_TYPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/SubsequenceInfo.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

DEFINE_ENUM_CONSTS_NO_EXTRA(SearchType, SEARCH_TYPE, false);

/** @brief Search result */
struct SearchResult {
    /** @brief Position within the dataset and length of the result */
    SubsequenceInfo m_subs_info;
    /** @brief Distance of the result to the query */
    Real m_distance;

    /**
     * @brief Less than operator
     *
     * @param other The other SearchResult to compare to
     * @return `true` if the distance of this result is less than the distance of the other result
     */
    bool operator<(const SearchResult &other) const {
        return m_distance < other.m_distance || (m_distance == other.m_distance && m_subs_info < other.m_subs_info);
    }
};

/** @brief List of search results and whether they are known to be exact */
struct SearchResults {
    /** @brief List of search results */
    vec<SearchResult> m_results;
    /** @brief Whether the results are known to be exact */
    bool m_exact = false;
};

#endif  // SEARCH_TYPE_HPP
