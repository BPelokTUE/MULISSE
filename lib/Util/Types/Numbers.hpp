#ifndef UTIL_TYPES_NUMBERS_HPP
#define UTIL_TYPES_NUMBERS_HPP

#include <cstdint>

using uint = uint32_t;
using SaxNumBitsT = uint8_t;
using SaxSegIndT = uint16_t;
using SaxSymbolT = uint16_t;
using MtsNumChannelsT = uint16_t;

#ifdef USE_DOUBLE
using Real = double;
#else
using Real = float;
#endif

#endif  // UTIL_TYPES_NUMBERS_HPP
