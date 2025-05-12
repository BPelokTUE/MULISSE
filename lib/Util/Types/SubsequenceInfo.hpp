#ifndef SUBSEQUENCE_INFO_HPP
#define SUBSEQUENCE_INFO_HPP

#include <cereal/access.hpp>
#include <sstream>

#include "Util/Types/Numbers.hpp"

struct SubsequenceInfo {
    /** @brief Index of the series within the file */
    uint m_series_ind;
    /** @brief Index of the start position of the subsequence within the series */
    uint m_start_pos;
    /** @brief Length of the subsequence */
    uint m_length;

    /**
     * @brief Less than operator
     * @param other The other SubsequenceInfo object to compare with
     * @return `true` if the series index of this is less or if their the same and the subsequence start of this is less
     */
    bool operator<(const SubsequenceInfo &other) const {
        return m_series_ind < other.m_series_ind ||
               (m_series_ind == other.m_series_ind && m_start_pos < other.m_start_pos);
    }

    /**
     * @brief Equality operator
     * @param other The other SubsequenceInfo object to compare with
     * @return `true` if the series index, start position and length of this and the other are equal
     */
    bool operator==(const SubsequenceInfo &other) const {
        return m_series_ind == other.m_series_ind && m_start_pos == other.m_start_pos && m_length == other.m_length;
    }

    /**
     * @brief Get the starting position of a subsequence in a file
     * @param series_len Length of the time series in the file
     * @param num_channels Number of channels in each time series
     * @param channel The index of the channel to get the position for
     * @return The starting position of the subsequence in the file
     */
    std::streampos get_file_pos(uint series_len, MtsNumChannelsT num_channels, MtsNumChannelsT channel = 0) const {
        return ((m_series_ind * num_channels + channel) * series_len + m_start_pos) * sizeof(Real);
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_series_ind, m_start_pos, m_length);
    }
};

#endif  // SUBSEQUENCE_INFO_HPP
