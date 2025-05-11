#ifndef UTIL_HELPERFUNCS_CONVERSION_HPP
#define UTIL_HELPERFUNCS_CONVERSION_HPP

#include "Util/Types/Numbers.hpp"

template <typename T>
constexpr Real R(T value) {
    return static_cast<Real>(value);
}

template <typename T>
constexpr uint U(T value) {
    return static_cast<uint>(value);
}

#endif  // UTIL_HELPERFUNCS_CONVERSION_HPP
