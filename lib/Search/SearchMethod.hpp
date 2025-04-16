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
    virtual SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                                 const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                                 const vec<uint> *real_query_inds = nullptr) const = 0;

   protected:
    /**
     * @brief Get the Paa values and the length of the query
     * @param query The multivariate query
     * @param segment_len The length of the segments
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @return A pair containing the Paa values and the length of the query
     */
    inline std::pair<vec<vec<Real>>, uint> get_query_paa_and_len(const vec<vec<Real>> &query, uint segment_len,
                                                                 const vec<uint> *real_query_inds = nullptr) const {
        vec<vec<Real>> query_paa(query.size());
        size_t query_len = 0;
        for (size_t c = 0; c < query.size(); ++c) {
            if (query[c].empty()) continue;
            query_len = std::max(query_len, query[c].size());

            if constexpr (QS) {
                vec<Real> unsorted_query_channel(query_len);
                for (uint i = 0; i < query_len; ++i) unsorted_query_channel[real_query_inds->at(i)] = query[c][i];
                query_paa[c] = paa(unsorted_query_channel, segment_len);
            } else {
                query_paa[c] = paa(query[c], segment_len);
            }
        }
        return {query_paa, query_len};
    }
};

#endif  // SEARCH_METHOD_HPP
