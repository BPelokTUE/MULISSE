#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <cstdint>

template <typename T>
using vec = std::vector<T>;

using UlisseEnvelope = std::pair<vec<float>, vec<float>>;

using SaxSplitIndT = uint16_t;

using SaxSymbolT = uint32_t;

using SaxNumBitsT = uint8_t;

#endif  // TYPEDEFS_HPP
