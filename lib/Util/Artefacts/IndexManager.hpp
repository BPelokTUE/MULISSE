#ifndef UTIL_ARTEFACTS_INDEXMANAGER_HPP
#define UTIL_ARTEFACTS_INDEXMANAGER_HPP

#include "Index/Sax/BreakpointStrategy/SaxBreakpointStrategy.hpp"
#include "Index/Sax/SaxBreakpoints.hpp"
#include "Util/Artefacts/Artifact.hpp"

struct GeneralIndexProperties;
struct ISpecificIndexProperties;
class MtsDataset;
class IndexLogger;
class ParamEstimatesLogger;

class IndexManager : public IArtifact {
    bool m_normalized;

    GeneralIndexProperties &m_general_index_props;
    ISpecificIndexProperties &m_specific_index_props;

    SaxBreakpoints m_sax_breakpoints;
    uptr<ISaxBreakpointStrategy> m_sax_breakpoint_strategy;

    std::optional<std::reference_wrapper<MtsDataset>> m_source_dataset;
    std::optional<std::reference_wrapper<IndexLogger>> m_index_logger;
    std::optional<std::reference_wrapper<ParamEstimatesLogger>> m_param_estimates_logger;

    void apply_in_archive(cereal::JSONInputArchive &ar) override;

    void apply_out_archive(cereal::JSONOutputArchive &ar) override;

    void initialize_sax_breakpoints(const SaxProperties &sax_props);

    EnvelopeIndexProperties &get_envelope_index_props();

   public:
    str get_meta_path() const override;

    /**
     * @brief Constructor that sets the manager up for index creation
     * @param dataset The dataset to index
     * @param general_index_props General index properties
     * @param specific_index_props Specific index properties
     * @param data_path Path to the data directory where the index will be saved
     */
    IndexManager(MtsDataset &dataset, const GeneralIndexProperties &general_index_props,
                 const ISpecificIndexProperties &specific_index_props, const str &data_path);

    /**
     * @brief Constructor that sets the manager up for index loading
     * @param data_path Path to the data directory where the index is located
     * @param meta_path Path to the meta file of the index
     */
    IndexManager(const str &data_path, const str &meta_path);

    void initialize_sax_breakpoints();

    void set_optimal_params();

    void estimate_params(const IndexGenOptions &gen_opts);

    void setup_segmentation_strategies();

    void estimate_gamma(const IndexGenOptions &gen_opts);

    void construct_index(IndexLogger &logger);
};

#endif  // UTIL_ARTEFACTS_INDEXMANAGER_HPP
