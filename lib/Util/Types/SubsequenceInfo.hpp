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

    bool operator<(const SubsequenceInfo &other) const {
        return m_series_ind < other.m_series_ind ||
               (m_series_ind == other.m_series_ind && m_start_pos < other.m_start_pos);
    }

    bool operator==(const SubsequenceInfo &other) const {
        return m_series_ind == other.m_series_ind && m_start_pos == other.m_start_pos && m_length == other.m_length;
    }

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
