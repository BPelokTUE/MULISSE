#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <cstdint>

template <typename T>
using vec = std::vector<T>;

// first = lower, second = upper
using UlisseEnvelope = std::pair<vec<float>, vec<float>>;

using SaxNumBitsT = uint8_t;
using SaxSegIndT = uint16_t;
using SaxSymbolT = uint32_t;
using MtsNumChannelsT = uint16_t;
using SaxSplitIndT = std::pair<SaxSegIndT, MtsNumChannelsT>;
const uint8_t DEFAULT_NUM_BIT_LIMIT = 12;

using FilePositionT = uint64_t;

#endif  // TYPEDEFS_HPP
