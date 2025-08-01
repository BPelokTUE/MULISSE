#include "Util/HelperFuncs/Conversion.hpp"

template <typename T>
constexpr Real R(T value) {
    return static_cast<Real>(value);
}

template <typename T>
constexpr uint U(T value) {
    return static_cast<uint>(value);
}

template <typename T>
constexpr str STR(T value) {
    return std::to_string(value);
}
