#ifndef UTIL_HELPERFUNCS_CONTAINERS_HPP
#define UTIL_HELPERFUNCS_CONTAINERS_HPP

#include "Util/Types/Containers.hpp"

template <typename T>
bool vec_contains(const vec<T>& vec, const T& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

template <typename T, size_t N>
bool arr_contains(const std::array<T, N>& arr, const T& value) {
    return std::find(arr.begin(), arr.end(), value) != arr.end();
}

#endif  // UTIL_HELPERFUNCS_CONTAINERS_HPP
