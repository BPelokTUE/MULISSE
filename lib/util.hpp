#ifndef UTIL_HPP
#define UTIL_HPP

#include "typedefs.hpp"

size_t get_dataset_size(const str dataset_path);

template <typename T>
vec<str> get_keys(const umap<str, T> map) {
    vec<str> keys;
    for (const auto &pair : map) {
        keys.push_back(pair.first);
    }
    return keys;
}

#endif  // UTIL_HPP
