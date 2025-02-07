#ifndef PAA_HPP
#define PAA_HPP

#include <vector>

#include "Util/typedefs.hpp"

/**
 * @brief Piecewise Aggregate Approximation (PAA) of a time series
 *
 * This function computes the Piecewise Aggregate Approximation (PAA) of a time series
 *
 * @param ts The time series to approximate
 * @param segment_len The length of each segment Assumed to be greater than 0
 * @return The PAA of the time series
 */
vec<float> paa(const vec<float> &ts, uint segment_len);

#endif  // PAA_HPP
