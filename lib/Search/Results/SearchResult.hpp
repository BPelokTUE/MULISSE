#ifndef SEARCH_RESULT_HPP
#define SEARCH_RESULT_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

/** @brief Search result */
struct SearchResult {
    /** @brief Position within the dataset and length of the result */
    SubsequencePosition m_subs_pos;
    /** @brief Distance of the result to the query */
    Real m_distance;

    /**
     * @brief Less than operator
     *
     * @param other The other SearchResult to compare to
     * @return `true` if the distance of this result is less than the distance of the other result
     */
    bool operator<(const SearchResult &other) const {
        return m_distance < other.m_distance || (m_distance == other.m_distance && m_subs_pos < other.m_subs_pos);
    }
};

/** @brief List of search results and whether they are known to be exact */
struct SearchResults {
    /** @brief List of search results */
    vec<SearchResult> m_results;
    /** @brief Whether the results are known to be exact */
    bool m_exact = false;
};

#endif  // SEARCH_RESULT_HPP
