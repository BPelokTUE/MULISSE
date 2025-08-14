#include "CLI/Subcommands/IndexingSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "CLI/Transformers.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/Indexing/Indexing.hpp"
#include "Util/HelperFuncs/Errors.hpp"

IndexingSubcommand::IndexingSubcommand(CLI::App &app) {
    auto index_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(INDEX), "Construct MULISSE index");

    m_segmentation_params = std::make_unique<SegmentationParams>();
    m_score_based_chss_params = std::make_unique<ScoreBasedChSSParams>();
    m_sax_params = std::make_unique<SaxParams>();
    m_isax_trie_params = std::make_unique<iSaxTrieParams>();
    m_env_grouping_params = std::make_unique<EnvelopeGroupingParams>();
    m_merger_params = std::make_unique<MergerParams>();

    index_subcommand->add_option("-i,--index", m_index_path, "Output index path")
        ->required()
        ->check(validators::file_is_writable);
    index_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Path to dataset meta file")
        ->required()
        ->check(validators::file_is_readable);
    index_subcommand->add_option("-f,--format", m_index_options.m_index_format, "Index format")
        ->transform(CLI::CheckedTransformer(STR_TO_ARCHIVE_TYPE, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand->add_option("-l,--l_min", m_index_options.m_l_min, "Minimum length of subsequences")
        ->required()
        ->check(validators::positive_int);
    index_subcommand->add_option("-L,--l_max", m_index_options.m_l_max, "Maximum length of subsequences")
        ->required()
        ->check(validators::positive_int);
    index_subcommand->add_option("-s,--num_segments", m_segmentation_params->m_num_segments, "Number of segments")
        ->check(validators::positive_int)
        ->capture_default_str();
    index_subcommand
        ->add_option("-G,--lg_segmentation_strategy", m_segmentation_params->m_lg_strategy_type,
                     "Length group segmentation strategy to use")
        ->transform(CLI::CheckedTransformer(STR_TO_LENGTH_GROUP_SEGMENTATION_STRATEGY, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand
        ->add_option("-C,--ch_segmentation_strategy", m_segmentation_params->m_ch_strategy_type,
                     "Channel segmentation strategy to use")
        ->transform(CLI::CheckedTransformer(STR_TO_CHANNEL_SEGMENTATION_STRATEGY, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand
        ->add_option("-S,--segmentation_strategy", m_segmentation_params->m_strategy_type,
                     "Segmentation strategy to use")
        ->transform(CLI::CheckedTransformer(STR_TO_SEGMENTATION_STRATEGY, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand
        ->add_option("-g,--l_per_group", m_index_options.m_l_per_group,
                     "Lengths per group, 0 by default, indicating no length-based grouping")
        ->check(validators::positive_int)
        ->capture_default_str();
    index_subcommand
        ->add_option(
            "--multi_chss_num_seg_file", m_segmentation_params->m_ch_num_seg_props_file,
            "Path to the file containing the proportions of segments per channel, use in MultiChSegmentationStrategy")
        ->check(validators::file_is_readable)
        ->capture_default_str();
    index_subcommand
        ->add_option("-E,--env_score_func_type", m_env_score_func_type,
                     "Envelope score function type to use in ScoreBasedChSegmentationStrategy")
        ->transform(transformers::get_str_to_enum(STR_TO_ENVELOPE_SCORES_TYPE))
        ->capture_default_str();
    index_subcommand
        ->add_option("--score_based_chss_sample_size", m_score_based_chss_params->m_sample_size,
                     "Size of the sample to use for estimating envelope statistics in ScoreBasedChSegmentationStrategy "
                     "implementations")
        ->check(validators::positive_int)
        ->capture_default_str();
    index_subcommand
        ->add_option("--score_based_chss_segment_len", m_score_based_chss_params->m_segment_len,
                     "Length of the segments to use for estimating envelope statistics in "
                     "ScoreBasedChSegmentationStrategy implementations")
        ->check(validators::positive_int)
        ->capture_default_str();
    index_subcommand
        ->add_option("-e,--score_based_chss_score_exp", m_score_based_chss_params->m_score_exp,
                     "Exponent to use for ScoreBasedChSegmentationStrategy  with IEnvelopeScoreFunc implementations")
        ->capture_default_str();
    index_subcommand->add_option(
        "-w,--env_stats_weights_file", m_score_based_chss_params->m_weights_file,
        "Path to the file containing the weights ScoreBasedChSegmentationStrategy for with EnvelopeStatsScoreFunc");
    index_subcommand->add_flag("--adapt", m_index_options.m_adapt, "Adapt the index properties to the dataset");
    index_subcommand->add_option("-I,--inserter_type", m_index_options.m_inserter_type, "Entry inserter type")
        ->transform(transformers::get_str_to_enum(STR_TO_ENTRY_INSERTER_TYPE))
        ->capture_default_str();
    index_subcommand->add_option("-M,--merger,--merger_type", m_merger_params->m_entry_merger_type, "Entry merger type")
        ->transform(CLI::CheckedTransformer(STR_TO_ENTRY_MERGER_TYPE, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand
        ->add_option(
            "--merger_num_bits", m_merger_num_bits,
            "Number of bits to use for SAX-based entry mergers. If not provided, takes the value of `num_bits_limit`.")
        ->check(validators::positive_int)
        ->check(validators::get_le_validator(MAX_NUM_BITS_LIMIT))
        ->capture_default_str();
    index_subcommand
        ->add_option("--size_limit", m_index_size_limit,
                     "Maximum size of FlatEnvelopeIndex as a ratio of the dataset size, 0 by default, meaning no limit")
        ->check(validators::non_negative_real)
        ->capture_default_str();
    index_subcommand
        ->add_option("--param_estimator_type,--pe_type", m_param_estimator_type,
                     "Type of EnvelopeParamEstimator to use")
        ->transform(transformers::get_str_to_enum(STR_TO_ENV_PARAM_ESTIMATOR_TYPE))
        ->capture_default_str();
    index_subcommand->add_flag(
        "--param_estimator_qt_examine_whole,--pe_qt_examine_whole", m_pe_qt_examine_whole,
        "Examine the whole series when a subsequence examination is performed in query_time param estimator");
    index_subcommand
        ->add_option("--param_estimator_qt_distance,--pe_qt_distance", m_pe_qt_distance_type,
                     "Distance measure to use in query time param estimator")
        ->transform(transformers::get_str_to_enum(STR_TO_DISTANCE_TYPE))
        ->capture_default_str();
    index_subcommand
        ->add_option("--param_estimator_config_gen_type,--pe_config_gen_type", m_env_config_gen_type,
                     "Type of configuration generator to use for the parameter estimator")
        ->transform(transformers::get_str_to_enum(STR_TO_ENV_CONFIG_GENERATOR_TYPE))
        ->capture_default_str();
    index_subcommand
        ->add_option("--param_estimator_step,--pe_step", m_estimator_sampling_params.m_ind_step,
                     "Step size for the envelope generation in EnvelopeSamplingParamEstimator")
        ->check(validators::positive_int);
    index_subcommand
        ->add_option("--param_estimator_num_queries,--pe_num_queries", m_estimator_sampling_params.m_num_queries,
                     "Number of queries to use for the envelope generation in EnvelopeSamplingParamEstimator")
        ->check(validators::positive_int);
    index_subcommand
        ->add_option("--param_estimator_sample_frac,--pe_sample_frac", m_estimator_sampling_params.m_sample_frac,
                     "Fraction of the dataset to sample for the envelope generation in EnvelopeSamplingParamEstimator")
        ->check(validators::zero_to_one_fraction);
    index_subcommand->add_option("--param_estimator_num_configs,--pe_num_configs", m_estimator_num_configs,
                                 "Number of configurations to generate for random envelope parameter estimation");
    index_subcommand
        ->add_option("--index_sample_frac", m_index_sample_frac,
                     "Fraction of the dataset to index, intended for testing, "
                     "defaults to 1.0, meaning that the whole dataset is indexed")
        ->check(validators::zero_to_one_fraction)
        ->capture_default_str();
    index_subcommand->add_flag("!--no_log_num_seg_per_ch", m_log_num_seg_per_ch,
                               "Do not log the number of segments per channel in the index");
    index_subcommand->add_flag("--log_num_seg_all", m_log_num_seg_all,
                               "Log the number of segments for all length groups and channels in the index.");

    index_subcommand->add_option("-t,--index_type", m_index_options.m_index_method, "Index type")
        ->transform(CLI::CheckedTransformer(STR_TO_SEARCH_METHOD_TYPE, CLI::ignore_case))
        ->capture_default_str();
    // Envelope
    index_subcommand
        ->add_option("--env_width_min_w_update", m_env_width_min_w_update,
                     "Minimum sufficient width update for EnvWidthChSegmentationStrategy with EnvelopeWidthScoreFunc ")
        ->check(validators::non_negative_real)
        ->capture_default_str();
    index_subcommand->add_option("-p,--pos_per_env", m_pos_per_env, "Positions per envelope")
        ->check(validators::positive_int)
        ->capture_default_str();
    // SAX
    index_subcommand->add_option("-b,--num_bits", m_sax_params->m_num_bits, "Number of bits for the SAX breakpoints")
        ->check(validators::positive_int)
        ->check(validators::get_le_validator(MAX_NUM_BITS_LIMIT))
        ->capture_default_str();
    index_subcommand
        ->add_option("-B,--breakpoint_strategy", m_sax_params->m_breakpoint_strategy_type, "Breakpoint strategy")
        ->transform(CLI::CheckedTransformer(STR_TO_ISAX_BREAKPOINT_STRATEGY, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand->add_option("--breakpoints", m_sax_params->m_breakpoints_file, "Path to breakpoints file")
        ->check(validators::file_is_readable)
        ->capture_default_str();
    // iSAX
    index_subcommand->add_option("--split_strategy", m_isax_trie_params->m_split_strategy_type, "Split strategy")
        ->transform(CLI::CheckedTransformer(STR_TO_ISAX_SPLIT_STRATEGY, CLI::ignore_case))
        ->capture_default_str();
    index_subcommand->add_flag("--merge_in_leaves,--isax_merge_in_leaves", m_isax_trie_params->m_merge_in_leaves,
                               "Merge entries in the leaves of the iSAX trie.");
    index_subcommand->add_flag("!--prefer_first_in_em,!--isax_prefer_first_in_em",
                               m_isax_trie_params->m_min_num_bits_on_tie,
                               "Prefer the first segment over the one with the minimum number of bits, in case of ties "
                               "in the split when using EntropyMaximizing strategy");
    index_subcommand
        ->add_option("--first_layer_bits", m_isax_trie_params->m_first_layer_num_bits,
                     "Starting number of bits in iSAX indexes")
        ->check(validators::positive_int)
        ->check(validators::get_le_validator(MAX_NUM_BITS_LIMIT))
        ->capture_default_str();
    index_subcommand
        ->add_option("--leaf_capacity", m_isax_trie_params->m_leaf_capacity, "Leaf capacity of iSAX trie index")
        ->check(validators::positive_int)
        ->capture_default_str();
    // TreeEnv
    index_subcommand->add_flag("--group_per_series", m_env_grouping_params->m_group_per_series,
                               "Group envelope entries only per series, not per dataset in TreeEnvelopeIndex");
    index_subcommand
        ->add_option("--bucket_size", m_isax_trie_params->m_leaf_capacity, "Bucket size of Envelope tree index")
        ->check(validators::get_ge_validator(2))
        ->capture_default_str();
    // VL-Env and TreeEnv
    index_subcommand->add_flag("--use_inv_sax", m_env_grouping_params->m_use_inv_sax_sorting,
                               "Use invSAX sorting before grouping envelopes in TreeEnvelopeIndex");
    // VL-Env
    index_subcommand
        ->add_option("--max_width_change", m_env_grouping_params->m_max_width_change,
                     "Maximum mean width change to allow in VarianceLimitingEnvelopeGrouper")
        ->check(validators::non_negative_real)
        ->capture_default_str();
}

void IndexingSubcommand::set_up_execution(const CommonOptions *common_opts) {
    // Index group specific parsing
    if (arr_contains(METHODS_W_ENVELOPE, m_index_options.m_index_method)) {
        // m_pos_per_env = std::min(m_pos_per_env, series_len - l_min + 1);
    }
    uptr<SaxParams> merger_sax_params = nullptr;
    if (arr_contains(MERGERS_W_SAX, m_entry_merger_type)) {
        if (arr_contains(METHODS_W_ISAX, m_index_options.m_index_method) &&
            m_merger_num_bits < m_sax_params->m_num_bits) {
            std::cout << "Warning: The number of bits for the SAX-based merger is less than the bit limit for "
                         "the iSAX trie. The merger will use "
                      << U(m_sax_params->m_num_bits) << " bits (instead of " << U(m_merger_num_bits) << ").\n";
            m_merger_num_bits = m_sax_params->m_num_bits;
        }
        merger_sax_params = std::make_unique<SaxParams>(m_sax_params);
        merger_sax_params->m_num_bits = m_merger_num_bits;
    }
    m_merger_params->m_merger_sax_params = merger_sax_params.get();

    // Score-based channel segmentation strategy
    if (m_segmentation_params->m_ch_strategy_type == ChannelSegmentationStrategyType::SCORE_BASED) {
        m_score_based_chss_params->m_normalized = !m_common_opts->m_raw;
        m_segmentation_params->m_score_based_chss_params = m_score_based_chss_params.get();
    }
    m_env_grouping_params->m_type = m_index_options.m_index_method;

    // Index parameters
    switch (m_index_options.m_index_method) {
        case ISAX_ENVELOPE:
        case ISAX_ENV_W_ENV:
        case ISAX_ENV_W_SAX_ENV:
            m_index_options.m_index_params = std::make_unique<iSaxEnvelopeIndexParams>(
                *m_segmentation_params.get(), *m_merger_params.get(), m_pos_per_env, *m_sax_params.get(),
                *m_isax_trie_params.get());
            break;
        case ISAX:
            m_index_options.m_index_params = std::make_unique<iSaxIndexParams>(
                *m_segmentation_params.get(), *m_merger_params.get(), *m_sax_params.get(), *m_isax_trie_params.get());
            break;
        case ENVELOPE:
            m_index_options.m_index_params = std::make_unique<EnvelopeIndexParams>(
                *m_segmentation_params.get(), *m_merger_params.get(), m_pos_per_env);
            break;
        case SAX_ENVELOPE:
            m_index_options.m_index_params = std::make_unique<SaxEnvelopeIndexParams>(
                *m_segmentation_params.get(), *m_merger_params.get(), m_pos_per_env, *m_sax_params.get());
            break;
        case TREE_ENVELOPE:
        case BUCKETING_ENVELOPE:
        case VL_ENVELOPE:
            m_index_options.m_index_params = std::make_unique<TreeEnvelopeIndexParams>(
                *m_segmentation_params.get(), *m_merger_params.get(), m_pos_per_env, *m_sax_params.get(),
                *m_env_grouping_params.get());
            break;
        case SEQUENTIAL_SCAN:
            throw std::runtime_error("Sequential scan does not require indexation\n");
    }

    // Configuration estimator parameters
    if (arr_contains(METHODS_W_ESTIMABLE_SIZE, m_index_options.m_index_method) && m_index_size_limit > 0) {
        uptr<EstimatorSamplingParams> estimator_sampling_params_ptr = nullptr;
        if (arr_contains(SAMPLING_ESTIMATOR_TYPES, m_param_estimator_type)) {
            m_estimator_sampling_params.m_seed = m_common_opts->m_seed;
            estimator_sampling_params_ptr = std::make_unique<EstimatorSamplingParams>(m_estimator_sampling_params);
        }
        uptr<EnvConfigGeneratorParams> envelope_config_gen_params_ptr = nullptr;
        if (m_param_estimator_type != NO_EST && m_env_config_gen_type == RANDOM) {
            envelope_config_gen_params_ptr = std::make_unique<RandomEnvConfigGeneratorParams>(
                m_estimator_num_configs, m_common_opts->m_seed, m_segmentation_params->m_num_segments);
        }
        m_index_options.m_estimator_params = std::make_unique<EstimatorParams>(
            m_pe_qt_examine_whole, m_index_size_limit, m_param_estimator_type, m_pe_qt_distance_type,
            m_env_config_gen_type, std::move(estimator_sampling_params_ptr), std::move(envelope_config_gen_params_ptr));
    }

    bool estimate_parameters =
        arr_contains(METHODS_W_ESTIMABLE_SIZE, m_index_options.m_index_method) && m_index_size_limit > R(0.0);
    bool use_length_groups = m_index_options.m_l_per_group > 0 || estimate_parameters;

    m_index_options.m_normalized = !m_common_opts->m_raw;     // Remove
    m_index_options.m_use_length_groups = use_length_groups;  // Is this needed?
    m_index_options.m_num_channels = 0;                       // TODO
    m_index_options.m_series_len = 0;                         // Remove
}

void IndexingSubcommand::validate_arguments() {
    if (arr_contains(METHODS_W_ISAX, m_index_options.m_index_method)) {
        if (m_isax_trie_params->m_leaf_capacity == 0) {
            throw get_required_missing_error("--leaf_capacity");
        }
        if (m_isax_trie_params->m_first_layer_num_bits > m_sax_params->m_num_bits) {
            throw std::runtime_error("The first layer number of bits (" +
                                     STR(m_isax_trie_params->m_first_layer_num_bits) +
                                     ") cannot be greater than the number of bits of the SAX breakpoints (" +
                                     STR(m_sax_params->m_num_bits) + ").");
        }
    }
    if (arr_contains(METHODS_W_ENVELOPE, m_index_options.m_index_method) && m_pos_per_env == 0) {
        throw get_required_missing_error("--pos_per_env");
    }
    if (arr_contains(METHODS_W_SAX, m_index_options.m_index_method) &&
        m_sax_params->m_breakpoint_strategy_type == FIXED) {
        // When using fixed breakpoints strategy, the breakpoints file must be provided and must contain sufficient
        // breakpoints
        if (m_sax_params->m_breakpoints_file.empty()) {
            throw get_required_missing_error("--breakpoints");
        }
        std::ifstream breakpoints_ifs(m_sax_params->m_breakpoints_file);
        SaxSegIndT alphabet_size = 1;
        Real breakpoint;
        while (breakpoints_ifs >> breakpoint) ++alphabet_size;

        if (alphabet_size < (1 << m_sax_params->m_num_bits)) {
            throw std::runtime_error("The file " + m_sax_params->m_breakpoints_file + " contains only " +
                                     STR(alphabet_size - 1) + " breakpoints, but " +
                                     STR((1 << m_sax_params->m_num_bits) - 1) +
                                     " are required. Provide a different file or lower the number of bits limit.");
        }
    }
    if (m_index_size_limit > R(0.0) && !arr_contains(METHODS_W_ESTIMABLE_SIZE, m_index_options.m_index_method)) {
        throw std::runtime_error("Index size limit is set, but the index type does not support size estimation.");
    }
};

void IndexingSubcommand::execute() {
    create_index(m_index_options, m_index_sample_frac, m_log_num_seg_per_ch, m_log_num_seg_all);
}
