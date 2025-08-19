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
     * @brief Constructor that initializes the dataset for generation
     * @param dataset_props Properties for the multivariate time series dataset
     * @param data_path Path to the data directory where the dataset will be saved
     */
    MtsDataset(const MtsDatasetProperties &dataset_props, const str &data_path);

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
