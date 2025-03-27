#ifndef DISTANCE_MEASURE_HPP
#define DISTANCE_MEASURE_HPP

#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Search/ResultSet.hpp"

/** @brief Types of distance measure */
enum DistanceType { ED, MASS };

DEFINE_ENUM_CONSTS(DistanceType, DISTANCE_TYPE, false, (umap<str, DistanceType>{{"euclidean", ED}}));

/** @brief Interface for distance measures */
class IDistanceMeasure {
   public:
    virtual ~IDistanceMeasure() = default;

    /**
     * @brief Calculate the minimum distance squared between a PAA value and a segment
     *
     * @param paa PAA value
     * @param lower Lower bound of the segment
     * @param upper Upper bound of the segment
     * @return Distance squared
     */
    virtual Real min_dist_squared(const Real paa, Real lower, Real upper) const = 0;

    /**
     * @brief Update the result set with the subsequences from the time series
     *
     * @param result_set Result set to update
     * @param subs_info Information about the subsequence in the dataset
     * @param query Query time series
     * @param mts Time series to update the result set with
     * @return true if the result set was updated, false otherwise
     */
    virtual bool update_result_set(IResultSet *result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                                   const vec<vec<Real>> &mts) = 0;

    /** @brief Get the type of the distance measure */
    virtual DistanceType get_type() const = 0;
};

class EuclideanDistance : public IDistanceMeasure {
   public:
    EuclideanDistance(bool m_normalized, bool use_early_abandoning = true);

    Real min_dist_squared(const Real paa, Real lower, Real upper) const override;

    bool update_result_set(IResultSet *result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                           const vec<vec<Real>> &mts) override;

    DistanceType get_type() const override;

    bool uses_early_abandoning() const;

   protected:
    bool m_normalized;

   private:
    bool m_use_early_abandoning;
};

class EuclideanDistanceWMass : public EuclideanDistance {
   public:
    EuclideanDistanceWMass(bool normalized);

    bool update_result_set(IResultSet *result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                           const vec<vec<Real>> &mts) override;

    DistanceType get_type() const override;

   private:
    vec<Real> calculate_dot_products(const vec<Real> &query_channel, const vec<Real> &mts_channel,
                                     SubsequenceInfo subs_info, MtsNumChannelsT channel_ind) const;
};

#endif  // DISTANCE_MEASURE_HPP
