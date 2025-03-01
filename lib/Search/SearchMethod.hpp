#ifndef SEARCH_METHOD_HPP
#define SEARCH_METHOD_HPP

#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

class ISearchMethod {
   public:
    virtual ~ISearchMethod() = default;

    /**
     * @brief Search for multivariate subsequence
     * @param query Multivariate subsequence to search for
     * @param search_options Search options
     * @param dataset_ifs Input file stream for the dataset
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &opts,
                                     std::ifstream &dataset_ifs) const = 0;
};

#endif  // SEARCH_METHOD_HPP
