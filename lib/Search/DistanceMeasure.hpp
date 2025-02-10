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
    virtual DistanceT min_dist_squared(const float paa, float lower, float upper) const = 0;

    /**
     * @brief Update the result set with the subsequences from the time series
     *
     * @param result_set Result set to update
     * @param query Query time series
     * @param mts Time series to update the result set with
     * @return true if the result set was updated, false otherwise
     */
    virtual bool update_result_set(IResultSet *result_set, FilePositionT file_pos, const vec<vec<float>> &query,
                                   const vec<vec<float>> &mts, const bool early_abandoning) = 0;

    /** @brief Get the type of the distance measure */
    virtual DistanceType get_type() const = 0;
};

class EuclideanDistance : public IDistanceMeasure {
   public:
    DistanceT min_dist_squared(const float paa, float lower, float upper) const override;

    bool update_result_set(IResultSet *result_set, FilePositionT file_pos, const vec<vec<float>> &query,
                           const vec<vec<float>> &mts, const bool early_abandoning) override;

    DistanceType get_type() const override;
};

class EuclideanDistanceWMass : public EuclideanDistance {
   public:
    bool update_result_set(IResultSet *result_set, FilePositionT file_pos, const vec<vec<float>> &query,
                           const vec<vec<float>> &mts, const bool early_abandoning) override;

    EuclideanDistanceWMass(bool normalized);

    DistanceType get_type() const override;

   private:
    vec<DistanceT> calculate_dot_products(const vec<DistanceT> &query_channel, const vec<DistanceT> &mts_channel,
                                          FilePositionT file_pos, MtsNumChannelsT channel_ind) const;

    bool m_normalized;
};

#endif  // DISTANCE_MEASURE_HPP
