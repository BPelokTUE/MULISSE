#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <limits>
#include <cassert>

#include "Util/typedefs.hpp"

const uint8_t MAX_NUM_BITS_LIMIT = sizeof(SaxSymbolT) * 8 - 1;
static_assert(MAX_NUM_BITS_LIMIT <= sizeof(SaxSymbolT) * 8, "MAX_NUM_BITS_LIMIT exceeds the size of SaxSymbolT");

const Real INF = std::numeric_limits<Real>::max();
const Real EPS = static_cast<Real>(1e-8);
const Real MIN_SUBS_SIGMA = static_cast<Real>(1e-3);

#endif  // CONSTANTS_HPP
