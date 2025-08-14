#ifndef UTIL_ARTEFACTS_SETTINGS_MTSDATASETSETTINGS_HPP
#define UTIL_ARTEFACTS_SETTINGS_MTSDATASETSETTINGS_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

struct MtsDatasetSettings {
    MtsNumChannelsT m_num_channels;
    uint m_series_len;
    uint m_num_series;
    str m_dataset_path;
};

#endif  // UTIL_ARTEFACTS_SETTINGS_MTSDATASETSETTINGS_HPP
