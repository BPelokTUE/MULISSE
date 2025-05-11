#ifndef UTIL_CONSTANTS_MATH_HPP
#define UTIL_CONSTANTS_MATH_HPP

#include <limits>

#include "Util/Types/Numbers.hpp"

const Real INF = std::numeric_limits<Real>::max();
const Real EPS = static_cast<Real>(1e-8);
const Real MIN_SUBS_SIGMA = static_cast<Real>(1e-3);

#endif  // UTIL_CONSTANTS_MATH_HPP
