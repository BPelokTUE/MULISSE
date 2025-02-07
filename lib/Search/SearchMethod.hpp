#ifndef SEARCH_METHOD_HPP
#define SEARCH_METHOD_HPP

#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"

class ISearchMethod {
   public:
    virtual ~ISearchMethod() = default;

    /**
     * @brief Search for multivariate subsequence
     *
     * @param query Multivariate subsequence to search for
     * @param search_options Search options
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &opts) const = 0;
};

#endif  // SEARCH_METHOD_HPP
