#ifndef UTIL_ARTEFACTS_MTSDATASET_HPP
#define UTIL_ARTEFACTS_MTSDATASET_HPP

#include "Util/Artefacts/Settings/MtsDatasetSettings.hpp"

class MultivariateTimeSeries;

class MtsDataset {
    friend class RandomWalkSubcommand;

   private:
    MtsDatasetSettings m_settings;

   public:
    /** @brief Default constructor */
    MtsDataset();

    /**
     * @brief Constructor that initializes the dataset with given settings
     * @param dataset_settings Settings for the multivariate time series dataset
     */
    MtsDataset(const MtsDatasetSettings &dataset_settings);

    /**
     * @brief Get the settings of the multivariate time series dataset
     * @return The settings of the multivariate time series dataset
     */
    const MtsDatasetSettings &get_settings() const;

    /**
     * @brief Get the path to the channel statistics file
     * @return The path to the channel statistics file
     */
    str get_channel_stats_path() const;

    /**
     * @brief Load a specific series from the dataset
     * @param series_index The index of the series to load
     * @return The loaded multivariate time series
     * @throws std::runtime_error if the series cannot be loaded
     */
    MultivariateTimeSeries load_series(uint series_index) const;
};

#endif  // UTIL_ARTEFACTS_MTSDATASET_HPP
