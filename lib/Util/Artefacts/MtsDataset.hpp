#ifndef UTIL_ARTEFACTS_MTSDATASET_HPP
#define UTIL_ARTEFACTS_MTSDATASET_HPP

#include <random>

#include "Util/Artefacts/Artifact.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Stats/ChannelStats.hpp"
#include "Util/Types/InputStream.hpp"
#include "Util/Types/OutputStream.hpp"
#include "Util/Types/Pointers.hpp"

class MultivariateTimeSeries;
struct RandomWalkGenOptions;
struct CsvDatasetGenOptions;

class MtsDataset : public IArtifact {
    friend class RandomWalkSubcommand;

    MtsDatasetProperties m_properties;
    ChannelStats m_channel_stats;

    InputStream m_istream;
    OutputStream m_ostream;

    /**
     * @brief Apply (save or load) the archive
     * @tparam Archive to use, JSONInputArchive or JSONOutputArchive
     * @param ar The archive to apply
     */
    template <typename Archive>
    void apply_archive(Archive &ar);

    void apply_in_archive(cereal::JSONInputArchive &ar) override;

    void apply_out_archive(cereal::JSONOutputArchive &ar) override;

   public:
    /** @brief Default constructor */
    MtsDataset();

    /**
     * @brief Constructor that initializes the dataset for generation
     * @param dataset_props Properties for the multivariate time series dataset
     * @param data_path Path to the data directory where the dataset will be saved
     */
    MtsDataset(const MtsDatasetProperties &dataset_props, const str &data_path);

    /**
     * @brief Constructor that initializes the dataset for intake
     * @param data_path Path to the data directory where the dataset is located
     * @param meta_path Path to the meta file of the dataset
     */
    MtsDataset(const str &data_path, const str &meta_path);

    str get_meta_path() const override;

    /**
     * @brief Get the properties of the multivariate time series dataset
     * @return The properties of the multivariate time series dataset
     */
    const MtsDatasetProperties &get_properties() const;

    /**
     * @brief Get the size of the dataset on disk
     * @return The size of the dataset on disk in bytes
     */
    size_t get_size_on_disk() const;

    /**
     * @brief Generate the random walk dataset
     * @param rw_gen_opts Options for generating the random walk dataset
     */
    void generate_random_walks(const RandomWalkGenOptions &rw_gen_opts);

    /**
     * @brief Generate the dataset from CSV files
     * @param csv_gen_opts Options for generating the dataset from CSV files
     */
    void generate_from_csvs(const CsvDatasetGenOptions &csv_gen_opts);

    /**
     * @brief Load a specific series from the dataset
     * @param series_index The index of the series to load
     * @param channel_mask Optional mask for channels to load, if empty all channels are loaded
     * @return The loaded multivariate time series
     * @throws std::runtime_error if the series cannot be loaded
     */
    MultivariateTimeSeries load_series(uint series_index, const vec<bool> &channel_mask = {});

    /**
     * @brief Load the next series from the dataset
     * @param channel_mask Optional mask for channels to load, if empty all channels are loaded
     * @return The loaded multivariate time series
     */
    MultivariateTimeSeries load_next_series(const vec<bool> &channel_mask = {});
};

#endif  // UTIL_ARTEFACTS_MTSDATASET_HPP
