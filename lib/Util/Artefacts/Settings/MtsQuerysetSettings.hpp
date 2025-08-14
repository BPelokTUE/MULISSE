#ifndef UTIL_ARTEFACTS_SETTINGS_MTSQUERYSETSETTINGS_HPP
#define UTIL_ARTEFACTS_SETTINGS_MTSQUERYSETSETTINGS_HPP

#include "Util/Types/LengthRange.hpp"
#include "Util/Types/Numbers.hpp"

struct MtsQuerysetSettings {
    /** @brief Number of queries to generate */
    uint m_num_queries;
    /** @brief Allowed query length range */
    LengthRange m_length_range;
    /** @brief Path to the queryset */
    str m_queryset_path;
};

#endif  // UTIL_ARTEFACTS_SETTINGS_MTSQUERYSETSETTINGS_HPP
