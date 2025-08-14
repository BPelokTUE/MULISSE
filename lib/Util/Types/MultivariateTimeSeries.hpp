#ifndef UTIL_TYPES_MULTIVARIATETIMESERIES_HPP
#define UTIL_TYPES_MULTIVARIATETIMESERIES_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

class MultivariateTimeSeries {
   protected:
    vec<vec<Real>> m_data;

   public:
    /**
     * @brief Construct MTS from data
     * @param data Multivariate time series data
     */
    MultivariateTimeSeries(const vec<vec<Real>> &&data);

    /**
     * @brief Indexing operator to access channels
     * @param channel_idx Index of the channel to access
     * @return Reference to the channel data
     */
    vec<Real> &operator[](size_t channel_idx);

    /**
     * @brief Read-only indexing operator to access channels
     * @param channel_idx Index of the channel to access
     * @return Const reference to the channel data
     */
    const vec<Real> &operator[](size_t channel_idx) const;
};

#endif  // UTIL_TYPES_MULTIVARIATETIMESERIES_HPP
