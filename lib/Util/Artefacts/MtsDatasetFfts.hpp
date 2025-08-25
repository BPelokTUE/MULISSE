#ifndef UTIL_ARTEFACTS_MTSDATASETFFTS_HPP
#define UTIL_ARTEFACTS_MTSDATASETFFTS_HPP

#include <functional>
#include <optional>

#include "Util/Artefacts/Artifact.hpp"
#include "Util/Types/InputStream.hpp"
#include "Util/Types/OutputStream.hpp"
#include "Util/Types/Vec.hpp"

class MtsDataset;
class FftArray;

class MtsDatasetFfts : public IArtifact {
    str m_ffts_path, m_source_dataset_path;

    std::optional<std::reference_wrapper<MtsDataset>> m_source_dataset;

    InputStream m_istream;
    OutputStream m_ostream;

    // (*2) for real and imaginary part, (*2) for extra components at the end
    static constexpr size_t FILE_SIZE_MULTIPLIER = 4;

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
    MtsDatasetFfts();

    /**
     * @brief Constructor that initializes FFTs for generation
     * @param ffts_path Path to the FFTs file to generate
     * @param dataset The source dataset
     */
    MtsDatasetFfts(const str &ffts_path, MtsDataset &dataset);

    str get_meta_path() const override;

    /** @brief Generate the MtsFfts */
    void generate();

    /**
     * @brief Load the FFTs for all channels of a specific series
     * @param series_index The index of the series to load FFTs for
     * @param channel_mask Optional mask for channels to load, if empty all channels are loaded
     */
    vec<FftArray> load_series_ffts(uint series_index, const vec<bool> &channel_mask = {});

    /**
     * @brief Get the source dataset of the FFTs
     * @return The source dataset
     */
    MtsDataset &get_source_dataset() const;

    /**
     * @brief Get the size of the FFTs on disk
     * @return The size of the FFTs on disk in bytes
     */
    size_t get_size_on_disk() const;

    /**
     * @brief Get the path to the FFTs file
     * @return The path to the FFTs file
     */
    const str &get_ffts_path() const;
};

#endif  // UTIL_ARTEFACTS_MTSDATASETFFTS_HPP
