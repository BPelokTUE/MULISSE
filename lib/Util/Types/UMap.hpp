#ifndef UTIL_TYPES_UMAP_HPP
#define UTIL_TYPES_UMAP_HPP

#include <unordered_map>

template <typename K, typename V>
using umap = std::unordered_map<K, V>;

template <typename K, typename V, typename H>
using umap_hash = std::unordered_map<K, V, H>;

#endif  // UTIL_TYPES_UMAP_HPP
