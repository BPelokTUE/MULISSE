#ifndef SEARCH_SEARCHMETHOD_HPP
#define SEARCH_SEARCHMETHOD_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"
#include "Search/DistanceMeasure/DistanceMeasure.hpp"
#include "Search/Results/SearchResult.hpp"
#include "Search/SearchOptions.hpp"

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
     * @param segmentation_strategy The segmentation strategy to use
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @return A pair containing the Paa values and the length of the query
     */
    inline std::pair<vec<vec<Real>>, uint> get_query_paa_and_len(const vec<vec<Real>> &query,
                                                                 const ISegmentationStrategy *segmentation_strategy,
                                                                 const vec<uint> *real_query_inds = nullptr) const {
        vec<vec<Real>> query_paa(query.size());
        size_t query_len = 0;

        for (size_t c = 0; c < query.size(); ++c) {
            if (query[c].empty()) continue;
            query_len = std::max(query_len, query[c].size());

            if constexpr (QS) {
                vec<Real> unsorted_query_channel(query_len);
                for (uint i = 0; i < query_len; ++i) unsorted_query_channel[real_query_inds->at(i)] = query[c][i];
                query_paa[c] = paa(unsorted_query_channel, segmentation_strategy);
            } else {
                query_paa[c] = paa(query[c], segmentation_strategy);
            }
        }
        return {query_paa, query_len};
    }
};

#endif  // SEARCH_SEARCHMETHOD.HPP
