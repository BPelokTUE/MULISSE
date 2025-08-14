#ifndef UTIL_RUNSETTINGS_DATASETPROPERTIES_HPP
#define UTIL_RUNSETTINGS_DATASETPROPERTIES_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

struct DatasetProperties {
    MtsNumChannelsT m_num_channels;
    uint m_series_len;
    uint m_num_series;
    str m_dataset_file;

    /**
     * @brief Get the path to the channel statistics file
     * @return The path to the channel statistics file
     */
    str get_channel_stats_path() const;
};

#endif  // UTIL_RUNSETTINGS_DATASETPROPERTIES_HPP
