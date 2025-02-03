#ifndef UTIL_HPP
#define UTIL_HPP

#include "typedefs.hpp"

/**
 * @brief Get the size of a dataset
 *
 * @param dataset_path Path to the dataset
 * @return The size of the dataset
 */
size_t get_dataset_size(const str dataset_path);

/**
 * @brief Get the keys of a map
 *
 * @tparam T Type of the map values
 * @param map The map
 * @return Vector of keys
 */
template <typename T>
vec<str> get_keys(const umap<str, T> map) {
    vec<str> keys;
    for (const auto &pair : map) {
        keys.push_back(pair.first);
    }
    return keys;
}

#endif  // UTIL_HPP
