#ifndef UTIL_ARTEFACTS_MTSQUERYSET_HPP
#define UTIL_ARTEFACTS_MTSQUERYSET_HPP

#include <fstream>

#include "Util/Artefacts/MetaArtifact.hpp"
#include "Util/Artefacts/Settings/MtsQuerysetSettings.hpp"
#include "Util/Types/MtsQuery.hpp"

class MtsQueryset : public MetaArtifact {
   private:
    MtsQuerysetSettings m_settings;
    std::ifstream m_queryset_ifs;

    /**
     * @brief Apply (save or load) the archive
     * @tparam Archive to use, JSONInputArchive or JSONOutputArchive
     * @param ar The archive to apply
     */
    template <typename Archive>
    void apply_archive(Archive &ar);

   public:
    /** @brief Default constructor */
    MtsQueryset();

    /**
     * @brief Constructor that initializes the queryset with given settings
     * @param queryset_settings Settings for the multivariate time series queryset
     */
    MtsQueryset(const MtsQuerysetSettings &queryset_settings);

    void save(const str &out_file, ArchiveType ar_type) override;

    void load(const str &out_file, ArchiveType ar_type) override;

    /**
     * @brief Get the settings of the multivariate time series queryset
     * @return The settings of the multivariate time series queryset
     */
    const MtsQuerysetSettings &get_settings() const;

    /**
     * @brief Get the path to the queryset meta file
     * @return The path to the queryset meta file
     */
    str get_meta_path() const;

    /**
     * @brief Load the next query from the queryset
     * @param normalized Whether to load the query normalized
     * @return The next query time series
     */
    MtsQuery load_next_query(bool normalized);
};

#endif  // UTIL_ARTEFACTS_MTSQUERYSET_HPP
