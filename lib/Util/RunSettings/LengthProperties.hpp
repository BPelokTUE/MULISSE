#ifndef UTIL_RUNSETTINGS_LENGTHPROPERTIES_HPP
#define UTIL_RUNSETTINGS_LENGTHPROPERTIES_HPP

#include "Util/Types/Numbers.hpp"

struct LengthProperties {
    bool m_use_length_groups;
    uint m_l_min;
    uint m_l_max;
    uint m_l_per_group;
    uint m_num_l_groups;
};

#endif  // UTIL_RUNSETTINGS_LENGTHPROPERTIES_HPP
