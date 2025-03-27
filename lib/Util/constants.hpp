#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <limits>
#include <cassert>

#include "Util/typedefs.hpp"

const uint8_t DEFAULT_NUM_BIT_LIMIT = 15;

static_assert(DEFAULT_NUM_BIT_LIMIT <= sizeof(SaxSymbolT) * 8, "DEFAULT_NUM_BIT_LIMIT exceeds the size of SaxSymbolT");

const Real INF = std::numeric_limits<Real>::max();
const Real EPS_F = 1e-8;
const Real MIN_SUBS_SIGMA = 1e-3;

#endif  // CONSTANTS_HPP
