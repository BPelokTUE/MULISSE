#ifndef UTIL_ARTEFACTS_MTSQUERYSET_HPP
#define UTIL_ARTEFACTS_MTSQUERYSET_HPP

#include <fstream>

#include "Util/Artefacts/MetaArtifact.hpp"
#include "Util/Artefacts/Properties/MtsQuerySetProperties.hpp"
#include "Util/Types/MtsQuery.hpp"

class MtsQuerySet : public MetaArtifact {
   private:
    MtsQuerySetProperties m_properties;
    std::ifstream m_query_set_ifs;

    /**
     * @brief Apply (save or load) the archive
     * @tparam Archive to use, JSONInputArchive or JSONOutputArchive
     * @param ar The archive to apply
     */
    template <typename Archive>
    void apply_archive(Archive &ar);

   public:
    /** @brief Default constructor */
    MtsQuerySet();

    /**
     * @brief Constructor that initializes the query_set with given properties
     * @param query_set_props Properties for the multivariate time series query_set
     */
    MtsQuerySet(const MtsQuerySetProperties &query_set_props);

    void save(const str &out_file, ArchiveType ar_type) override;

    void load(const str &out_file, ArchiveType ar_type) override;

    /**
     * @brief Get the properties of the multivariate time series query_set
     * @return The properties of the multivariate time series query_set
     */
    const MtsQuerySetProperties &get_properties() const;

    /**
     * @brief Get the path to the query_set meta file
     * @return The path to the query_set meta file
     */
    str get_meta_path() const;

    /**
     * @brief Load the next query from the query_set
     * @param normalized Whether to load the query normalized
     * @return The next query time series
     */
    MtsQuery load_next_query(bool normalized);
};

#endif  // UTIL_ARTEFACTS_MTSQUERYSET_HPP
