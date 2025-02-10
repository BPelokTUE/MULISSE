#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <cassert>
#include <cstdint>

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

using FilePositionT = uint64_t;

using DistanceT = double;

#endif  // TYPEDEFS_HPP
