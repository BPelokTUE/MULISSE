#ifndef UTIL_CONSTANTS_MATH_HPP
#define UTIL_CONSTANTS_MATH_HPP

#include <limits>

#include "Util/Types/Numbers.hpp"

constexpr Real INF = std::numeric_limits<Real>::max();
constexpr Real EPS = static_cast<Real>(1e-8);
constexpr Real DEFAULT_MIN_SUBS_SD = static_cast<Real>(1e-3);
constexpr size_t RESULT_SET_MAX_CAPACITY = 250;

#endif  // UTIL_CONSTANTS_MATH_HPP
