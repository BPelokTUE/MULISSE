#ifndef DISTANCE_MEASURE_HPP
#define DISTANCE_MEASURE_HPP

#include "Search/ResultSet.hpp"

/** @brief Interface for distance measures */
class IDistanceMeasure {
   public:
    virtual ~IDistanceMeasure() = default;

    /**
     * @brief Update the result set with the subsequences from the time series
     *
     * @param result_set Result set to update
     * @param query Query time series
     * @param ts Time series to update the result set with
     * @return true if the result set was updated, false otherwise
     */
    virtual bool update_result_set(uptr<const IResultSet> result_set, const vec<float> &query,
                                   const vec<float> &ts) = 0;

    /**
     * @brief Calculate the minimum distance squared between a PAA value and a segment
     *
     * @param paa PAA value
     * @param lower Lower bound of the segment
     * @param upper Upper bound of the segment
     * @return Distance squared
     */
    virtual DistanceT min_dist_squared(const float paa, float lower, float upper) const = 0;
};

class EuclideanDistance : public IDistanceMeasure {
   public:
    bool update_result_set(uptr<const IResultSet> result_set, const vec<float> &query, const vec<float> &ts) override {
        return false;
    };

    DistanceT min_dist_squared(const float paa, float lower, float upper) const override { return 0; };
};

#endif  // DISTANCE_MEASURE_HPP
