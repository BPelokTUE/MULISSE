#ifndef SUBSEQUENCE_INFO_HPP
#define SUBSEQUENCE_INFO_HPP

#include "Util/Types/SubsequencePosition.hpp"

struct SubsequenceInfo {
    /** @brief Position of the subsequence */
    SubsequencePosition m_position;
    /** @brief Length of the subsequence */
    uint m_length;

    /**
     * @brief Less than operator
     * @param other The other SubsequenceInfo object to compare with
     * @return `true` if the series index of this is less or if their the same and the subsequence start of this is less
     */
    bool operator<(const SubsequenceInfo &other) const { return m_position < other.m_position; }

    /**
     * @brief Equality operator
     * @param other The other SubsequenceInfo object to compare with
     * @return `true` if the series index, start position and length of this and the other are equal
     */
    bool operator==(const SubsequenceInfo &other) const {
        return m_position == other.m_position && m_length == other.m_length;
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_position, m_length);
    }
};

#endif  // SUBSEQUENCE_INFO_HPP
