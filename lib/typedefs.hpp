#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <cstdint>

template <typename T>
using vec = std::vector<T>;

using UlisseEnvelope = std::pair<vec<float>, vec<float>>;

using iSaxSplitIndT = uint16_t;

using iSaxSymbolT = uint32_t;

using iSaxNumBitsT = uint8_t;

#endif  // TYPEDEFS_HPP
