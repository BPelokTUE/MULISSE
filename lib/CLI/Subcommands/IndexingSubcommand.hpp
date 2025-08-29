#ifndef CLI_SUBCOMMANDS_INDEXINGSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_INDEXINGSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Enums/ArchiveType.hpp"
#include "Enums/IndexType.hpp"
#include "Util/Artefacts/Options/IndexGenOptions.hpp"
#include "Util/Artefacts/Properties/GeneralIndexProperties.hpp"
#include "Util/Artefacts/Properties/SpecificIndexProperties.hpp"
#include "Util/Constants/Sax.hpp"
#include "Util/Types/String.hpp"

class IndexingSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    IndexingSubcommand(CLI::App &app);

    void execute(const RunContext &run_context) override;

   private:
    /**
     * @brief Parse specific index properties
     * @param dataset The dataset to index
     * @param run_context Context for the run
     */
    void parse_specific_index_props(const MtsDataset &dataset, const RunContext &run_context);

    /**
     * @brief Get the specific index properties
     * @return The specific index properties
     */
    uptr<ISpecificIndexProperties> get_specific_index_props();

    /**
     * @brief Parse the estimator parameters
     * @param run_context Context for the run
     */
    void parse_index_gen_options(const RunContext &run_context);

    bool m_pe_qt_examine_whole = false, m_log_num_seg_per_ch = false, m_log_num_seg_all = false;

    SaxNumBitsT m_merger_num_bits = MAX_NUM_BITS_LIMIT;

    uint m_pos_per_env = 0, m_estimator_num_configs = 0;

    Real m_index_sample_frac = R(1.0), m_env_width_min_w_update = R(0.0), m_index_size_limit = R(0.0);

    // paths
    str m_index_path, m_dataset_meta_path;

    // index properties and gen options
    IndexGenOptions m_index_gen_opts;

    GeneralIndexProperties m_general_index_props;

    // specific index property components
    SegmentationProperties m_segmentation_props;
    SaxProperties m_sax_props;
    iSaxTrieProperties m_isax_trie_props;
    EnvelopeGroupingProperties m_env_grouping_props;
    ScoreBasedChSSParams m_score_based_chss_params;
    MergerProperties m_merger_props;

    EnvelopeScoresType m_env_score_func_type = EnvelopeScoresType::WIDTH;
    str m_env_stats_weights_file = "";

    EntryMergerType m_entry_merger_type = EntryMergerType::DUMMY;

    // parameter estimation
    EnvelopeParamEstimatorType m_param_estimator_type = EnvelopeParamEstimatorType::ONLY_GAMMA;
    EnvelopeConfigGeneratorType m_env_config_gen_type = EnvelopeConfigGeneratorType::RANDOM;
    DistanceType m_pe_qt_distance_type = DistanceType::ED;

    EstimatorSamplingParams m_estimator_sampling_params{
        .m_ind_step = 10,
        .m_num_queries = 100,
        .m_sample_frac = R(0.05),
    };
};

#endif  // CLI_SUBCOMMANDS_INDEXINGSUBCOMMAND_HPP
