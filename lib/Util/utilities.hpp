#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <sstream>

#include "magic_enum/magic_enum.hpp"

#include "Util/typedefs.hpp"
#include "Util/constants.hpp"

// Dataset

/**
 * @brief Get the size of a dataset; TODO: this should be in `RunSettings`
 * @param dataset_path Path to the dataset
 * @return The size of the dataset
 */
inline size_t get_dataset_size(const str dataset_path) {
    std::ifstream data_stream(dataset_path, std::ios::binary | std::ios::ate);
    return data_stream.tellg();
}

// Helper

template <typename T>
bool vec_contains(const vec<T>& vec, const T& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

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
    umap<str, T> string_to_enum_map;
    // Add lower case mappings
    for (auto e : magic_enum::enum_values<T>()) {
        str name(magic_enum::enum_name(e));
        // Add lower case -> enum value
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        string_to_enum_map.emplace(name, e);
    }
    // Add acronyms
    if (add_acronyms) {
        for (auto e : magic_enum::enum_values<T>()) {
            str name(magic_enum::enum_name(e));
            // Lower case name
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
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
    }
    // Add extra mappings
    for (const auto& pair : extra_mappings) {
        string_to_enum_map.emplace(pair.first, pair.second);
    }

    return string_to_enum_map;
}

/**
 * @brief Generate enum to string map
 *
 * Generate a map containing (Enum value -> lower case enum name) mappings
 *
 * @tparam T Enum type
 * @return Map from enum value to string
 */
template <typename T>
umap<T, str> generate_enum_to_string_map() {
    umap<T, str> enum_to_string_map;
    for (auto e : magic_enum::enum_values<T>()) {
        str name(magic_enum::enum_name(e));
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        enum_to_string_map.emplace(e, name);
    }
    return enum_to_string_map;
}

/**
 * @brief Get the values of an enum
 * @tparam T Type of the map values
 */
template <typename T>
vec<T> get_enum_values() {
    auto val_array = magic_enum::enum_values<T>();
    return vec<T>(val_array.begin(), val_array.end());
}

/**
 * @brief Get the values of an enum to string map
 * @tparam T Enum type
 * @param map The map
 * @return Vector of strings
 */
template <typename T>
vec<str> get_enum_strings(const umap<T, str> map) {
    vec<str> strings;
    vec<T> values = get_enum_values<T>();
    for (T val : values) strings.push_back(map.at(val));
    return strings;
}

/**
 * @brief Get the accepted strings of a map
 * @tparam T Type of the map values
 * @param map The map
 * @return Vector of accepted strings
 */
template <typename T>
vec<str> get_accepted_strings(const umap<str, T> map) {
    vec<str> strings;
    for (const auto& pair : map) strings.push_back(pair.first);
    return strings;
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
 * - STR_TO_<ENUM_NAME>: Map from string to enum type. See generate_string_to_enum_map.
 * - <ENUM_NAME>_TO_STR: Map from enum type to string. See generate_enum_to_string_map.
 * - <ENUM_NAME>_STRS: Vector of lowercase enum names. See get_enum_strings.
 * - ACCEPTED_<ENUM_NAME>_STRS: Vector of accepted strings for STR_TO_<ENUM_NAME>. See get_accepted_strings.
 * - <ENUM_NAME>_ENUMS: Vector of enum values. See get_enum_values.
 *
 * @param ENUM_TYPE Enum type
 * @param ENUM_NAME Name of the enum
 * @param GENERATE_ACRONYM Whether to generate acronyms
 * @param EXTRA_MAPPINGS Extra mappings to add to the map
 */
#define DEFINE_ENUM_CONSTS(ENUM_TYPE, ENUM_NAME, GENERATE_ACRONYM, EXTRA_MAPPINGS)                           \
    inline const umap<str, ENUM_TYPE> STR_TO_##ENUM_NAME =                                                   \
        generate_string_to_enum_map<ENUM_TYPE>(GENERATE_ACRONYM, EXTRA_MAPPINGS);                            \
    inline const umap<ENUM_TYPE, str> ENUM_NAME##_TO_STR = generate_enum_to_string_map<ENUM_TYPE>();         \
    inline const vec<str> ENUM_NAME##_STRS = get_enum_strings<ENUM_TYPE>(ENUM_NAME##_TO_STR);                \
    inline const vec<str> ACCEPTED_##ENUM_NAME##_STRS = get_accepted_strings<ENUM_TYPE>(STR_TO_##ENUM_NAME); \
    inline const vec<ENUM_TYPE> ENUM_NAME##_ENUMS = get_enum_values<ENUM_TYPE>();

/** @brief Same as DEFINE_ENUM_CONSTS with no extra mappings */
#define DEFINE_ENUM_CONSTS_NO_EXTRA(ENUM_TYPE, ENUM_NAME, GENERATE_ACRONYM) \
    DEFINE_ENUM_CONSTS(ENUM_TYPE, ENUM_NAME, GENERATE_ACRONYM, (umap<str, ENUM_TYPE>{}))

// Parallelism

#ifndef DISABLE_PARALLELISM
#define OMP_PRAGMA(x) _Pragma(#x)
#else
#define OMP_PRAGMA(x)
#endif

// Math

/**
 * @brief Calculate the mean and standard deviation from the sum, sum of squares and count
 * @param sum Sum of the values
 * @param sum_sq Sum of the squares of the values
 * @param count Number of values
 */
inline std::pair<Real, Real> calculate_mu_and_sigma(Real sum, Real sum_sq, uint count) {
    Real mu = sum / count;
    Real sigma = std::sqrt(std::max(sum_sq / count - mu * mu, EPS_F));
    return {mu, sigma};
}

#endif  // UTILITIES_HPP
