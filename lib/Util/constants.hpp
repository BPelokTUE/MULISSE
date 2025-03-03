#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <limits>
#include <cassert>

#include "Util/typedefs.hpp"

const uint8_t DEFAULT_NUM_BIT_LIMIT = 15;

static_assert(DEFAULT_NUM_BIT_LIMIT <= sizeof(SaxSymbolT) * 8, "DEFAULT_NUM_BIT_LIMIT exceeds the size of SaxSymbolT");

const float INF = std::numeric_limits<float>::max();
// const float EPS_F = std::numeric_limits<float>::epsilon();
// const double EPS = std::numeric_limits<double>::epsilon();
const float EPS_F = 1e-8;
const double EPS = 1e-8;
const double MIN_SUBS_SIGMA = 1e-3;

#endif  // CONSTANTS_HPP
