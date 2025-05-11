#ifndef UTIL_RUNSETTINGS_DATASETPROPERTIES_HPP
#define UTIL_RUNSETTINGS_DATASETPROPERTIES_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

struct DatasetProperties {
    MtsNumChannelsT m_num_channels;
    uint m_series_len;
    uint m_num_series;
    str m_file;
};

#endif  // UTIL_RUNSETTINGS_DATASETPROPERTIES_HPP
