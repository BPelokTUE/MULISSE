#ifndef UTIL_ARTEFACTS_MTSQUERYSET_HPP
#define UTIL_ARTEFACTS_MTSQUERYSET_HPP

#include <fstream>
#include <functional>
#include <optional>

#include "Util/Artefacts/MetaArtifact.hpp"
#include "Util/Artefacts/Properties/MtsQuerySetProperties.hpp"
#include "Util/Types/MtsQuery.hpp"
#include "Util/Types/Pointers.hpp"

class MtsDataset;
struct QuerySetGenOptions;

class MtsQuerySet : public MetaArtifact {
   private:
    MtsQuerySetProperties m_properties;

    std::optional<std::reference_wrapper<MtsDataset>> m_source_dataset;

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
    MtsQuerySet();

    /**
     * @brief Constructor that initializes the query set for generation
     * @param dataset The source dataset
     * @param query_set_props Properties for the multivariate time series query_set
     * @param ostream Output stream to write the query set to
     */
    MtsQuerySet(MtsDataset &dataset, const MtsQuerySetProperties &query_set_props, uptr<std::ostream> ostream);

    void save(const str &out_file, ArchiveType ar_type) override;

    void load(const str &out_file, ArchiveType ar_type) override;

    str get_meta_path() const override;

    /**
     * @brief Get the properties of the multivariate time series query_set
     * @return The properties of the multivariate time series query_set
     */
    const MtsQuerySetProperties &get_properties() const;

    /**
     * @brief Set the source dataset for the query set
     * @param source_dataset Pointer to the source dataset
     */
    void set_source_dataset(MtsDataset &source_dataset);

    /**
     * @brief Get the source dataset of the query set
     * @return Pointer to the source dataset
     */
    MtsDataset &get_source_dataset() const;

    /**
     * @brief Set the input stream of the query set
     * @param istream The input stream
     * @throws std::runtime_error if the stream cannot be opened
     */
    void set_istream(uptr<std::istream> istream);

    /**
     * @brief Set the output stream of the query set
     * @param ostream The output stream
     * @throws std::runtime_error if the stream cannot be opened
     */
    void set_ostream(uptr<std::ostream> ostream);

    /**
     * @brief Load the next query from the query_set
     * @param normalized Whether to load the query normalized
     * @return The next query time series
     */
    MtsQuery load_next_query(bool normalized);

    /**
     * @brief Generate the query set
     * @param query_set_gen_opts Options for generating the query set
     * @param series_inds Optional list of time series indices to use for generating the queries. If empty, a random
     * subset of time series will be used.
     */
    void generate(const QuerySetGenOptions &query_set_gen_opts, const vec<uint> &series_inds = {});
};

#endif  // UTIL_ARTEFACTS_MTSQUERYSET_HPP
