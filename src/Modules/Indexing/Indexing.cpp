#include "Modules/Indexing/Indexing.hpp"

#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"
#include "Modules/Indexing/ConstructIndex.hpp"
#include "Modules/Indexing/GetChannelScores.hpp"
#include "Modules/Indexing/GetEntryGenerator.hpp"
#include "Modules/Indexing/GetEntryMerger.hpp"
#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetISaxIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetTreeEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetTwoStageEnvelopeIndex.hpp"
#include "Modules/Indexing/InitializeBreakpoints.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"

int create_index(const IndexOptions &opts, Real sample_frac, bool log_num_seg_per_ch, bool log_num_seg_all) {
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();
    str index_path = RS.get_index_path();

    if (!std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " does not exist." << std::endl;
        return 1;
    }

    IndexLogger::initialize(opts, sample_frac);
    auto &logger = IndexLogger::get_instance();
    logger.start_timer(ISC::INDEXING_TIME_S);

    // Initialized SAX breakpoints

    if (arr_contains(METHODS_W_ISAX, opts.m_index_method)) {
        auto *index_params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());
        if (index_params && index_params->m_isax_trie_params.m_num_bits_limit > 0) {
            initialize_sax_breakpoints(index_params->m_sax_params, index_params->m_isax_trie_params.m_num_bits_limit);
        } else {
            std::cerr << "iSAX index method requires a positive number of bits limit\n";
            return 2;
        }
    } else if (arr_contains(METHODS_W_SAX, opts.m_index_method)) {
        auto *index_params = dynamic_cast<SaxIndexParams *>(opts.m_index_params.get());
        if (index_params && index_params->m_sax_params.m_num_bits > 0) {
            initialize_sax_breakpoints(index_params->m_sax_params, index_params->m_sax_params.m_num_bits);
        } else {
            std::cerr << "SAX index method requires a positive number of bits\n";
            return 2;
        }
    } else if (arr_contains(METHODS_W_PAA, opts.m_index_method)) {
        auto *index_params = dynamic_cast<PaaIndexParams *>(opts.m_index_params.get());
        if (index_params && arr_contains(MERGERS_W_SAX, index_params->m_merger_params.m_entry_merger_type)) {
            auto merger_sax_params = index_params->m_merger_params.m_merger_sax_params;
            if (merger_sax_params && merger_sax_params->m_num_bits > 0) {
                initialize_sax_breakpoints(*merger_sax_params, merger_sax_params->m_num_bits);
            } else {
                std::cerr << "SAX-based envelope entry merger requires a positive number of bits\n";
                return 2;
            }
        }
    }

    // Estimate approximately optimal parameters if requested
    Real index_size_limit = opts.m_estimator_params ? opts.m_estimator_params->m_index_size_limit : R(0.0);

    auto envelope_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
    if (opts.m_estimator_params) {
        if (!envelope_params) {
            std::cerr << "FlatEnvelopeIndex requires EnvelopeIndexParams.\n";
            return 2;
        }
        if (envelope_params->m_segmentation_params.m_lg_strategy_type == ADAPTIVE_MULTI) {
            std::cerr << "Index size estimation is not supported for AdaptiveMultiSegmentationStrategy.\n";
            return 2;
        }
    }

    if (opts.m_estimator_params && opts.m_estimator_params->m_estimate_parameters) {
        auto estimator = FlatEnvelopeParamEstimator(opts);
        auto flat_envelope_params = estimator.get_estimated_params();
        RS.set_flat_envelope_params(flat_envelope_params);
        logger.set_flat_envelope_params(flat_envelope_params);
        envelope_params->set_flat_envelope_params(flat_envelope_params);
    }

    // Set up segmentation strategies

    logger.start_timer(ISC::SEGMENTATION_SETUP_TIME_S);
    vec<Real> channel_scores = get_channel_scores(opts);
    auto lg_segmentation_strategy = get_lg_segmentation_strategy(opts, channel_scores);
    logger.set_num_segments_cols(lg_segmentation_strategy.get(), log_num_seg_per_ch, log_num_seg_all);
    logger.stop_timer(ISC::SEGMENTATION_SETUP_TIME_S);

    // Set pos_per_env based on required index size, if applicable

    if (opts.m_estimator_params && !opts.m_estimator_params->m_estimate_parameters) {
        uint pos_per_env = get_max_pos_per_env(index_size_limit, lg_segmentation_strategy.get());
        size_t estimated_size = get_estimated_flat_envelope_size(pos_per_env, lg_segmentation_strategy.get()),
               size_limit = static_cast<size_t>(index_size_limit * R(get_dataset_size(dataset_path)));

        if (estimated_size > size_limit) {
            std::cerr << "Size limit is insufficient for requested parameters. "
                      << "Estimated size of the index is " << estimated_size << " bytes, "
                      << "but the limit is " << size_limit << " bytes." << std::endl;
            return 3;
        }
        RS.set_pos_per_env(pos_per_env);
        logger.set_pos_per_env(pos_per_env);
        envelope_params->m_pos_per_env = pos_per_env;
    }

    // Create index

    IndexFactoryParams factory_params{
        .m_discretize_flat_index = false,
        .m_opts = opts,
    };

#define CONSTRUCT_INDEX(Type, index_factory)                                                                          \
    construct_index<Type>(                                                                                            \
        [](IndexFactoryParams &factory_params_in) -> sptr<IIndex<Type>> { return index_factory(factory_params_in); }, \
        std::move(generator), std::move(merger), factory_params, std::move(lg_segmentation_strategy), sample_frac);

#define CONSTRUCT_ENVELOPE_INDEX(index_factory)                                    \
    auto generator = get_envelope_generator(opts, lg_segmentation_strategy.get()); \
    auto merger = get_entry_merger<Envelope>(opts);                                \
    CONSTRUCT_INDEX(Envelope, index_factory);

#define CONSTRUCT_PAA_INDEX(index_factory)                                    \
    auto generator = get_paa_generator(opts, lg_segmentation_strategy.get()); \
    auto merger = get_entry_merger<Paa>(opts);                                \
    CONSTRUCT_INDEX(Paa, index_factory);

    switch (opts.m_index_method) {
        case ISAX_ENVELOPE: {
            CONSTRUCT_ENVELOPE_INDEX(get_isax_index<Envelope>);
            break;
        }
        case ISAX_ENV_W_SAX_ENV:
            factory_params.m_discretize_flat_index = true;
        case ISAX_ENV_W_ENV: {
            CONSTRUCT_ENVELOPE_INDEX(get_two_stage_isax_envelope_index);
            break;
        }
        case ISAX: {
            CONSTRUCT_PAA_INDEX(get_isax_index<Paa>);
            break;
        }
        case SAX_ENVELOPE:
            factory_params.m_discretize_flat_index = true;
        case ENVELOPE: {
            CONSTRUCT_ENVELOPE_INDEX(get_flat_envelope_index);
            break;
        }
        case TREE_ENVELOPE:
        case BUCKETING_ENVELOPE:
        case VL_ENVELOPE: {
            CONSTRUCT_ENVELOPE_INDEX(get_envelope_tree_index);
            break;
        }
        default:
            break;
    }
    logger.stop_timer(ISC::INDEXING_TIME_S);

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
        logger.increment_count_col(ISC::SIZE_ON_DISK_B, RS.get_ffts_size_on_disk());
    }

    logger.write_entry();

    return 0;
}
