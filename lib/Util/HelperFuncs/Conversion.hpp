#ifndef UTIL_HELPERFUNCS_CONVERSION_HPP
#define UTIL_HELPERFUNCS_CONVERSION_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

template <typename T>
constexpr Real R(T value);

template <typename T>
constexpr uint U(T value);

template <typename T>
constexpr str STR(T value);

#endif  // UTIL_HELPERFUNCS_CONVERSION_HPP
