#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <memory>
#include <string>
#include <limits>
#include <unordered_map>
#include <cstdint>
#include <cassert>

template <typename T>
using vec = std::vector<T>;

template <typename K, typename V>
using umap = std::unordered_map<K, V>;

template <typename T>
using uptr = std::unique_ptr<T>;

using str = std::string;

using uint = uint32_t;
using SaxNumBitsT = uint8_t;
using SaxSegIndT = uint16_t;
using SaxSymbolT = uint16_t;
using MtsNumChannelsT = uint16_t;
using SaxSplitIndT = std::pair<SaxSegIndT, MtsNumChannelsT>;
const uint8_t DEFAULT_NUM_BIT_LIMIT = 12;

static_assert(DEFAULT_NUM_BIT_LIMIT <= sizeof(SaxSymbolT) * 8, "DEFAULT_NUM_BIT_LIMIT exceeds the size of SaxSymbolT");

const float INF = std::numeric_limits<float>::max();
const float EPS_F = std::numeric_limits<float>::epsilon();
const double EPS = std::numeric_limits<double>::epsilon();

using FilePositionT = uint64_t;

using DistanceT = double;

#endif  // TYPEDEFS_HPP
