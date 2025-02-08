#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <sstream>

#include "magic_enum/magic_enum.hpp"

#include "Util/typedefs.hpp"

// Dataset

/**
 * @brief Get the size of a dataset; TODO: this should be in `RunSettings`
 *
 * @param dataset_path Path to the dataset
 * @return The size of the dataset
 */
size_t get_dataset_size(const str dataset_path);

// Enums

/**
 * @brief Generate string to enum map
 *
 * Generate a map from string to enum type, with the following mappings:
 * - Lower case enum name -> enum value
 * - Acronym of enum name -> enum value (if `add_acronyms` is true)
 * - Extra mappings (if provided)
 *
 * @tparam T Enum type
 * @param add_acronyms Whether to add acronyms to the map. Acronyms are assumed to be unique, in case of a collision
 *        only the last acronym will be kept.
 * @param extra_mappings Extra mappings to add to the map
 */
template <typename T>
umap<str, T> generate_string_to_enum_map(bool add_acronyms = false, umap<str, T> extra_mappings = {}) {
    umap<str, T> string_to_enum_map(extra_mappings);
    for (auto e : magic_enum::enum_values<T>()) {
        str name(magic_enum::enum_name(e));
        // Add lower case -> enum value
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        string_to_enum_map.emplace(name, e);

        if (!add_acronyms) continue;

        // Add acronym -> enum value
        str acronym;
        std::stringstream ss(name);
        str token;
        while (std::getline(ss, token, '_')) {
            if (!token.empty()) {
                acronym += token[0];
            }
        }
        string_to_enum_map.emplace(acronym, e);
    }
    return string_to_enum_map;
}

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

/**
 * @brief Define enum constants
 *
 * Defines the following constants for the given enum type:
 * - STR_TO_<ENUM_TYPE>: Map from string to enum type. See `generate_string_to_enum_map`.
 * - <ENUM_TYPE>_STRS: Vector of accepted strings for STR_TO_<ENUM_TYPE>.
 * - <ENUM_TYPE>_VALUES: Vector of enum values.
 *
 * @param ENUM_TYPE Enum type
 * @param ENUM_NAME Name of the enum
 * @param GENERATE_ACRONYM Whether to generate acronyms
 * @param EXTRA_MAPPINGS Extra mappings to add to the map
 */
#define DEFINE_ENUM_CONSTS(ENUM_TYPE, ENUM_NAME, GENERATE_ACRONYM, EXTRA_MAPPINGS) \
    inline const umap<str, ENUM_TYPE> STR_TO_##ENUM_NAME =                         \
        generate_string_to_enum_map<ENUM_TYPE>(GENERATE_ACRONYM, EXTRA_MAPPINGS);  \
    inline const vec<str> ENUM_NAME##_STRS = get_map_keys(STR_TO_##ENUM_NAME);     \
    inline const auto ENUM_NAME##_VALUES = magic_enum::enum_values<ENUM_TYPE>();

/** @brief Same as DEFINE_ENUM_CONSTS with no extra mappings */
#define DEFINE_ENUM_CONSTS_NO_EXTRA(ENUM_TYPE, ENUM_NAME, GENERATE_ACRONYM) \
    DEFINE_ENUM_CONSTS(ENUM_TYPE, ENUM_NAME, GENERATE_ACRONYM, (umap<str, ENUM_TYPE>{}))

// Math

std::pair<float, float> calculate_mu_and_sigma(float sum, float sum_sq, uint count);

std::pair<double, double> calculate_mu_and_sigma(double sum, double sum_sq, uint count);

#endif  // UTILITIES_HPP
