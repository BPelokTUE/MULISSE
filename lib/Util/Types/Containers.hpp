#ifndef UTIL_TYPES_STR_HPP
#define UTIL_TYPES_STR_HPP

#include <string>
#include <unordered_map>
#include <vector>

template <typename T>
using vec = std::vector<T>;

template <typename K, typename V>
using umap = std::unordered_map<K, V>;

template <typename K, typename V, typename H>
using umap_hash = std::unordered_map<K, V, H>;

using str = std::string;

#endif  // UTIL_TYPES_STR_HPP
