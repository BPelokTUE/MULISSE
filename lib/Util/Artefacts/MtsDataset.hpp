#ifndef UTIL_ARTEFACTS_MTSDATASET_HPP
#define UTIL_ARTEFACTS_MTSDATASET_HPP

#include <random>

#include "Util/Artefacts/MetaArtifact.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Stats/ChannelStats.hpp"
#include "Util/Types/Pointers.hpp"

class MultivariateTimeSeries;
struct RandomWalkGenOptions;
struct CsvDatasetGenOptions;

class MtsDataset : public MetaArtifact {
    friend class RandomWalkSubcommand;

   private:
    MtsDatasetProperties m_properties;
    ChannelStats m_channel_stats;

    uptr<std::istream> m_istream;
    uptr<std::ostream> m_ostream;

    /**
     * @brief Apply (save or load) the archive
     * @tparam Archive to use, JSONInputArchive or JSONOutputArchive
     * @param ar The archive to apply
     */
    template <typename Archive>
    void apply_archive(Archive &ar);

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
     * @brief Set the output stream of the dataset
     * @param ostream The output stream
     * @throws std::runtime_error if the stream cannot be opened
     */
    void set_ostream(uptr<std::ostream> ostream);

    /**
     * @brief Set the input stream of the dataset
     * @param istream The input stream
     * @throws std::runtime_error if the stream cannot be opened
     */
    void set_istream(uptr<std::istream> istream);

    /**
     * @brief Set up dataset generation
     * @param data_path Path to the data directory where the dataset will be saved
     */
    void set_up_generation(const str &data_path);

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
