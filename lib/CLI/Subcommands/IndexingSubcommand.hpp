#ifndef CLI_SUBCOMMANDS_INDEXINGSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_INDEXINGSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Enums/ArchiveType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Index/IndexOptions.hpp"
#include "Util/Constants/Sax.hpp"
#include "Util/Types/String.hpp"

class IndexingSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    IndexingSubcommand(CLI::App &app);

    void set_up_execution(const CommonOptions *common_opts) override;

    void validate_arguments() override;

    void execute() override;

   private:
    bool m_pe_qt_examine_whole = false, m_log_num_seg_per_ch = false, m_log_num_seg_all = false;

    SaxNumBitsT m_merger_num_bits = MAX_NUM_BITS_LIMIT;

    uint m_pos_per_env = 0, m_estimator_num_configs = 0;

    Real m_index_sample_frac = R(1.0), m_env_width_min_w_update = R(0.0), m_index_size_limit = R(0.0);

    // paths
    str m_index_path, m_dataset_meta_path;

    // index options and parameters
    IndexOptions m_index_options{
        .m_index_method = SearchMethodType::SAX_ENVELOPE,
        .m_index_format = ArchiveType::BINARY,
        .m_inserter_type = EntryInserterType::PARALLEL,
    };

    uptr<SegmentationParams> m_segmentation_params = nullptr;
    uptr<SaxParams> m_sax_params = nullptr;
    uptr<iSaxTrieParams> m_isax_trie_params = nullptr;
    uptr<EnvelopeGroupingParams> m_env_grouping_params = nullptr;
    uptr<ScoreBasedChSSParams> m_score_based_chss_params = nullptr;
    uptr<MergerParams> m_merger_params = nullptr;

    EnvelopeScoresType m_env_score_func_type = EnvelopeScoresType::WIDTH;
    str m_env_stats_weights_file = "";

    EntryMergerType m_entry_merger_type = EntryMergerType::DUMMY;

    // parameter estimation
    EnvelopeParamEstimatorType m_param_estimator_type = EnvelopeParamEstimatorType::NO_EST;
    EnvelopeConfigGeneratorType m_env_config_gen_type = EnvelopeConfigGeneratorType::RANDOM;
    DistanceType m_pe_qt_distance_type = DistanceType::ED;

    EstimatorSamplingParams m_estimator_sampling_params{
        .m_ind_step = 10,
        .m_num_queries = 100,
        .m_sample_frac = R(0.05),
    };
};

#endif  // CLI_SUBCOMMANDS_INDEXINGSUBCOMMAND_HPP
