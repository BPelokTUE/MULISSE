#ifndef SEARCH_METHOD_HPP
#define SEARCH_METHOD_HPP

#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Search/DistanceMeasure.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/**
 * @brief Interface for search methods
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 */
template <SearchType S, DistanceType D>
class ISearchMethod {
   public:
    virtual ~ISearchMethod() = default;

    /**
     * @brief Search for multivariate subsequence
     * @param query Multivariate subsequence to search for
     * @param search_options Search options
     * @param result_set Set for managing the results
     * @param distance_measure Distance measure to use
     * @param dataset_ifs Input file stream for the dataset
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<SearchResult> search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                                     const DistanceMeasure<S, D> &distance_measure,
                                     std::ifstream &dataset_ifs) const = 0;
};

#endif  // SEARCH_METHOD_HPP
