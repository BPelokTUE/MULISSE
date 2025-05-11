#ifndef UTIL_HELPERFUNCS_MATH_HPP
#define UTIL_HELPERFUNCS_MATH_HPP

#include <cmath>
#include <utility>

#include "Util/Constants/Math.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Calculate the mean and standard deviation from the sum, sum of squares and count
 * @tparam T Type of the values
 * @param sum Sum of the values
 * @param sum_sq Sum of the squares of the values
 * @param count Number of values
 */
template <typename T>
inline std::pair<T, T> calculate_mu_and_sigma(T sum, T sum_sq, uint count) {
    T count_t = static_cast<T>(count);
    T mu = sum / count_t;
    T sigma = std::sqrt(std::max(sum_sq / count_t - mu * mu, static_cast<T>(EPS)));
    return {mu, sigma};
}

#endif  // UTIL_HELPERFUNCS_MATH_HPP
