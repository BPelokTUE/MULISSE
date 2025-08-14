#ifndef UTIL_ARTEFACTS_PROPERTIES_MTSDATASETPROPERTIES_HPP
#define UTIL_ARTEFACTS_PROPERTIES_MTSDATASETPROPERTIES_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

/** @brief Properties of a MtsDataset */
struct MtsDatasetProperties {
    /** @brief Number of channels of the time series in the dataset */
    MtsNumChannelsT m_num_channels;
    /** @brief Length of the time series in the dataset */
    uint m_series_len;
    /** @brief Number of series in the dataset */
    uint m_num_series;
    /** @brief Path to the dataset file */
    str m_dataset_path;
};

#endif  // UTIL_ARTEFACTS_PROPERTIES_MTSDATASETPROPERTIES_HPP
