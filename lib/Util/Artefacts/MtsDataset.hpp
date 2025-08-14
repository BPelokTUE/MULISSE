#ifndef UTIL_ARTEFACTS_MTSDATASET_HPP
#define UTIL_ARTEFACTS_MTSDATASET_HPP

#include <fstream>

#include "Util/Artefacts/MetaArtifact.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"

class MultivariateTimeSeries;

class MtsDataset : public MetaArtifact {
    friend class RandomWalkSubcommand;

   private:
    MtsDatasetProperties m_properties;

    std::ifstream m_dataset_ifs;

    /**
     * @brief Apply (save or load) the archive
     * @tparam Archive to use, JSONInputArchive or JSONOutputArchive
     * @param ar The archive to apply
     */
    template <typename Archive>
    void apply_archive(Archive &ar);

    /**
     * @brief Open the input file stream for the dataset
     * @throws std::runtime_error if the dataset file cannot be opened
     */
    void open_ifs();

   public:
    /** @brief Default constructor */
    MtsDataset();

    /**
     * @brief Constructor that initializes the dataset with given properties
     * @param dataset_props Properties for the multivariate time series dataset
     */
    MtsDataset(const MtsDatasetProperties &dataset_props);

    void save(const str &out_file, ArchiveType ar_type) override;

    void load(const str &out_file, ArchiveType ar_type) override;

    /**
     * @brief Get the properties of the multivariate time series dataset
     * @return The properties of the multivariate time series dataset
     */
    const MtsDatasetProperties &get_properties() const;

    /**
     * @brief Get the path to the channel statistics file
     * @return The path to the channel statistics file
     */
    str get_channel_stats_path() const;

    /**
     * @brief Get the path to the dataset meta file
     * @return The path to the dataset meta file
     */
    str get_meta_path() const;

    /**
     * @brief Get the size of the dataset on disk
     * @return The size of the dataset on disk in bytes
     */
    size_t get_size_on_disk() const;

    /**
     * @brief Load a specific series from the dataset
     * @param series_index The index of the series to load
     * @return The loaded multivariate time series
     * @throws std::runtime_error if the series cannot be loaded
     */
    MultivariateTimeSeries load_series(uint series_index);

    /**
     * @brief Load the next series from the dataset
     * @return The loaded multivariate time series
     */
    MultivariateTimeSeries load_next_series();
};

#endif  // UTIL_ARTEFACTS_MTSDATASET_HPP
