#ifndef SEARCH_TYPE_HPP
#define SEARCH_TYPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

DEFINE_ENUM_CONSTS_NO_EXTRA(SearchType, SEARCH_TYPE, false);

/** @brief Search result */
struct SearchResult {
    /** @brief Position within the dataset and length of the result */
    SubsequenceInfo subs_info;
    /** @brief Distance of the result to the query */
    Real distance;

    /**
     * @brief Less than operator
     *
     * @param other The other SearchResult to compare to
     * @return `true` if the distance of this result is less than the distance of the other result
     */
    bool operator<(const SearchResult &other) const {
        return distance < other.distance || (distance == other.distance && subs_info < other.subs_info);
    }
};

#endif  // SEARCH_TYPE_HPP
