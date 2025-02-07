#ifndef UTIL_HPP
#define UTIL_HPP

#include "Util/typedefs.hpp"

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
 * @tparam V Type of the map keys
 * @param map The map
 * @return Vector of keys
 */
template <typename K, typename V>
vec<K> get_map_keys(const umap<K, V> map) {
    vec<K> keys;
    for (const auto& pair : map) {
        keys.push_back(pair.first);
    }
    return keys;
}

/**
 * @brief Get the inverse of a map
 *
 * @tparam K Type of the map values
 * @tparam V Type of the map keys
 * @param map The map
 * @return Inverse of the map
 */
template <typename K, typename V>
umap<V, K> get_inverse_map(const umap<K, V> map) {
    umap<V, K> inverse_map;
    for (const auto& pair : map) {
        inverse_map[pair.second] = pair.first;
    }
    return inverse_map;
}

std::pair<float, float> calculate_mu_and_sigma(float sum, float sum_sq, uint count);

std::pair<double, double> calculate_mu_and_sigma(double sum, double sum_sq, uint count);

#endif  // UTIL_HPP
