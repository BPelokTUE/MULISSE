#ifndef UTIL_RUNSETTINGS_LENGTHPROPERTIES_HPP
#define UTIL_RUNSETTINGS_LENGTHPROPERTIES_HPP

#include <algorithm>

#include "Util/Types/Numbers.hpp"

struct LengthProperties {
    bool m_use_length_groups;
    uint m_l_min;
    uint m_l_max;
    uint m_l_per_group;
    uint m_num_l_groups;

    inline uint get_length_group(uint subs_length) const { return (subs_length - m_l_min) / m_l_per_group; }

    inline uint get_lg_l_min(uint lg_ind) const {
        return m_use_length_groups ? m_l_min + lg_ind * m_l_per_group : m_l_min;
    }

    inline uint get_lg_l_max(uint lg_ind) const {
        return m_use_length_groups ? std::min(m_l_max, m_l_min + (lg_ind + 1) * m_l_per_group - 1) : m_l_max;
    }
};

#endif  // UTIL_RUNSETTINGS_LENGTHPROPERTIES_HPP
