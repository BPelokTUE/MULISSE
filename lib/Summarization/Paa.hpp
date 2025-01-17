#ifndef PAA_HPP
#define PAA_HPP

#include <vector>

/**
 * @brief Piecewise Aggregate Approximation (PAA) of a time series.
 *
 * This function computes the Piecewise Aggregate Approximation (PAA) of a time series.
 *
 * @param ts The time series to approximate.
 * @param segment_len The length of each segment.
 * @return The PAA of the time series.
 */
std::vector<float> paa(const std::vector<float> &ts, unsigned segment_len);

#endif  // PAA_HPP
