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
 * @tparam QS Whether the query is sorted or not
 */
template <SearchType S, DistanceType D, bool QS = false>
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
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<SearchResult> search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                                     const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                                     const vec<uint> *real_query_inds = nullptr) const = 0;
};

#endif  // SEARCH_METHOD_HPP
