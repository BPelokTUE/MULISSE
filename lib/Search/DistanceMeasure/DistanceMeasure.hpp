#ifndef SEARCH_DISTANCEMEASURE_DISTANCEMEASURE_HPP
#define SEARCH_DISTANCEMEASURE_DISTANCEMEASURE_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Search/Results/ResultSet.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Interface for distance measures
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 * */
template <SearchType S, DistanceType D, bool QS = false>
class DistanceMeasure {
   public:
    /** @brief Get the type of the distance measure */
    DistanceType get_type() const { return D; };

    /**
     * @brief Update the result set with the subsequences from the time series
     * @param result_set Result set to update
     * @param subs_info Information about the subsequence in the dataset
     * @param query Query time series
     * @param mts Time series to update the result set with
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @return true if the result set was updated, false otherwise
     */
    bool update_result_set(ResultSet<S> &result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                           const vec<vec<Real>> &mts, const vec<uint> *real_query_inds = nullptr) const;

    /**
     * @brief Calculate the minimum distance squared between a PAA value and a segment
     * @param paa PAA value
     * @param lower Lower bound of the segment
     * @param upper Upper bound of the segment
     * @return Distance squared
     */
    Real min_dist_squared(const Real paa, Real lower, Real upper) const;
};

#endif  // SEARCH_DISTANCEMEASURE_DISTANCEMEASURE_HPP
