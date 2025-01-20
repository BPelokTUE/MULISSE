#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <cstdint>

template <typename T>
using vec = std::vector<T>;

// first = lower, second = upper
using UlisseEnvelope = std::pair<vec<float>, vec<float>>;

using SaxNumBitsT = uint8_t;
using SaxSplitIndT = uint16_t;
using SaxSymbolT = uint32_t;
const uint8_t MAX_SYMBOL_BITS = sizeof(SaxSymbolT) * 8;  // Assuming that 1 byte is 8 bits

using FilePositionT = uint64_t;

#endif  // TYPEDEFS_HPP
