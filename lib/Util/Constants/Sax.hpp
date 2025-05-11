#ifndef UTIL_CONSTANTS_SAX_HPP
#define UTIL_CONSTANTS_SAX_HPP

#include <cassert>

#include "Util/Types/Numbers.hpp"

const uint8_t MAX_NUM_BITS_LIMIT = sizeof(SaxSymbolT) * 8 - 1;
static_assert(MAX_NUM_BITS_LIMIT <= sizeof(SaxSymbolT) * 8, "MAX_NUM_BITS_LIMIT exceeds the size of SaxSymbolT");

#endif  // UTIL_CONSTANTS_SAX_HPP
