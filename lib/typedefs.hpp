#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <cstdint>
#include <cassert>

template <typename T>
using vec = std::vector<T>;

using SaxNumBitsT = uint8_t;
using SaxSegIndT = uint16_t;
using SaxSymbolT = uint16_t;
using MtsNumChannelsT = uint16_t;
using SaxSplitIndT = std::pair<SaxSegIndT, MtsNumChannelsT>;
const uint8_t DEFAULT_NUM_BIT_LIMIT = 12;

static_assert(DEFAULT_NUM_BIT_LIMIT <= sizeof(SaxSymbolT) * 8, "DEFAULT_NUM_BIT_LIMIT exceeds the size of SaxSymbolT");

using FilePositionT = uint64_t;

#endif  // TYPEDEFS_HPP
