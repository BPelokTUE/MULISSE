#ifndef UTIL_TYPES_SUBSEQUENCEPOSITION_HPP
#define UTIL_TYPES_SUBSEQUENCEPOSITION_HPP

#include <boost/functional/hash.hpp>
#include <cereal/access.hpp>
#include <sstream>

struct SubsequencePosition {
    /** @brief Index of the series within the file */
    uint m_series;
    /** @brief Index of the start position of the subsequence within the series */
    uint m_start;

    /**
     * @brief Less than operator
     * @param other The other SubsequencePosition object to compare with
     * @return `true` if the series index of this is less or if they're the same and the subsequence start of this is
     * less
     */
    bool operator<(const SubsequencePosition &other) const {
        return m_series < other.m_series || (m_series == other.m_series && m_start < other.m_start);
    }

    /**
     * @brief Equality operator
     * @param other The other SubsequencePosition object to compare with
     * @return `true` if the series index and start position of this and the other are equal
     */
    bool operator==(const SubsequencePosition &other) const {
        return m_series == other.m_series && m_start == other.m_start;
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_series, m_start);
    }
};

struct SubsequencePositionHash {
    size_t operator()(const SubsequencePosition &position) const {
        size_t seed = 0;
        boost::hash_combine(seed, position.m_series);
        boost::hash_combine(seed, position.m_start);
        return seed;
    }
};

#endif  // UTIL_TYPES_SUBSEQUENCEPOSITION_HPP
