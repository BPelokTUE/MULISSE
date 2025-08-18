#ifndef UTIL_TYPES_LENGTHRANGE_HPP
#define UTIL_TYPES_LENGTHRANGE_HPP

#include "Util/Types/Numbers.hpp"

/** @brief Represents a range of allowed lengths for queries. */
struct LengthRange {
    uint m_l_min;
    uint m_l_max;

    /**
     * @brief Validate the length range.
     * @throws std::invalid_argument if m_l_min is greater than m_l_max.
     */
    void validate() const;

    template <typename Archive>
    void serialize(Archive &ar);
};

#endif  // UTIL_TYPES_LENGTHRANGE_HPP
