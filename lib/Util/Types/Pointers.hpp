#ifndef UTIL_TYPES_SPTR_HPP
#define UTIL_TYPES_SPTR_HPP

#include <memory>

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using sptr = std::shared_ptr<T>;

#endif  // UTIL_TYPES_SPTR_HPP
