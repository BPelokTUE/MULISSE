#include "Modules/Indexing/Indexing.hpp"

#include <cmath>

#include "Index/Entry/SaxEnvelope.hpp"
#include "Modules/Indexing/ConstructIndex.hpp"
#include "Modules/Indexing/EstimateEnvelopeParams.hpp"
#include "Modules/Indexing/GetChannelScores.hpp"
#include "Modules/Indexing/GetEntryGenerator.hpp"
#include "Modules/Indexing/GetEntryMerger.hpp"
#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetISaxIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetTreeEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetTwoStageEnvelopeIndex.hpp"
#include "Modules/Indexing/InitializeBreakpoints.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"

int create_index(IndexOptions &opts, Real index_sample_frac, bool log_num_seg_per_ch, bool log_num_seg_all) {
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();
    str index_path = RS.get_index_path();

    if (!std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " does not exist." << std::endl;
        return 1;
    }

    IndexLogger::initialize(opts, index_sample_frac);
    auto &logger = IndexLogger::get_instance();
    logger.start_timer(ISC::INDEXING_TIME_S);

    // Initialized SAX breakpoints

    auto paa_index_params = dynamic_cast<PaaIndexParams *>(opts.m_index_params.get());
    if (arr_contains(METHODS_W_ISAX, opts.m_index_method)) {
        auto *index_params = dynamic_cast<const iSaxIndexParams *>(opts.m_index_params.get());
        if (index_params && index_params->m_sax_params.m_num_bits > 0) {
            initialize_sax_breakpoints(index_params->m_sax_params);
        } else {
            std::cerr << "iSAX index method requires a positive number of bits limit\n";
            return 2;
        }
    } else if (arr_contains(METHODS_W_SAX, opts.m_index_method)) {
        auto *index_params = dynamic_cast<const SaxIndexParams *>(opts.m_index_params.get());
        if (index_params && index_params->m_sax_params.m_num_bits > 0) {
            initialize_sax_breakpoints(index_params->m_sax_params);
        } else {
            std::cerr << "SAX index method requires a positive number of bits\n";
            return 2;
        }
    } else if (arr_contains(METHODS_W_PAA, opts.m_index_method)) {
        if (paa_index_params && arr_contains(MERGERS_W_SAX, paa_index_params->m_merger_params.m_entry_merger_type)) {
            auto merger_sax_params = paa_index_params->m_merger_params.m_merger_sax_params;
            if (merger_sax_params && merger_sax_params->m_num_bits > 0) {
                initialize_sax_breakpoints(*merger_sax_params);
            } else {
                std::cerr << "SAX-based envelope entry merger requires a positive number of bits\n";
                return 2;
            }
        }
    }

    auto env_index_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
    if (opts.m_estimator_params && !env_index_params) {
        std::cerr << "FlatEnvelopeIndex requires EnvelopeIndexParams.\n";
        return 2;
    }

    // Set num_segments dynamically if required
    uint series_len = RS.get_dataset_props().m_series_len;
    if (paa_index_params && paa_index_params->m_segmentation_params.m_num_segments == 0) {
        // Calculate optimal number of segments if requested
        Real multiplier = opts.m_normalized
                              ? (paa_index_params->m_segmentation_params.m_strategy_type == UNIFORM ? 16.0 : 12.0)
                              : 8.0;
        SaxSegIndT num_segments =
            static_cast<SaxSegIndT>(std::ceil(std::sqrt(R(series_len) / R(2 * opts.m_l_min)) * multiplier));

        logger.set_num_segments(num_segments);
        opts.set_num_segments(num_segments);
    }
    // Set pos_per_env dynamically if required
    if (env_index_params && env_index_params->m_pos_per_env == 0) {
        // Use optimal positions per envelope if requested
        uint pos_per_env = opts.m_normalized ? 64 : 256;
        RS.set_pos_per_env(pos_per_env);
        logger.set_pos_per_env(pos_per_env);
        opts.set_pos_per_env(pos_per_env);
    }

    if (opts.m_estimator_params) {
        // Estimate flat envelope parameters if requested
        std::optional<EnvelopeParams> estimated_params;
        if (opts.m_estimator_params->m_qt_distance_type == ED) {
            if (opts.m_estimator_params->m_qt_examine_whole) {
                estimated_params = estimate_envelope_params<ED, true>(opts);
            } else {
                estimated_params = estimate_envelope_params<ED, false>(opts);
            }
        } else {  // D == MASS
            if (opts.m_estimator_params->m_qt_examine_whole) {
                estimated_params = estimate_envelope_params<MASS, true>(opts);
            } else {
                estimated_params = estimate_envelope_params<MASS, false>(opts);
            }
        }
        if (estimated_params) {
            RS.set_flat_envelope_params(*estimated_params);
            logger.set_flat_envelope_params(*estimated_params);
            opts.set_flat_envelope_params(*estimated_params);
        }
    }

    // Set up segmentation strategies
    logger.start_timer(ISC::SEGMENTATION_SETUP_TIME_S);
    vec<Real> channel_scores = get_channel_scores(opts);
    auto lg_segmentation_strategy = get_lg_segmentation_strategy(opts, &channel_scores);
    logger.set_num_segments_cols(lg_segmentation_strategy.get(), log_num_seg_per_ch, log_num_seg_all);
    logger.stop_timer(ISC::SEGMENTATION_SETUP_TIME_S);

#define CONSTRUCT_INDEX(Type, index_factory)                                                                          \
    construct_index<Type>(                                                                                            \
        [](IndexFactoryParams &factory_params_in) -> sptr<IIndex<Type>> { return index_factory(factory_params_in); }, \
        std::move(generator), std::move(merger), opts, std::move(lg_segmentation_strategy), index_sample_frac);

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
        case ISAX_ENV_W_SAX_ENV: {
            CONSTRUCT_ENVELOPE_INDEX(get_two_stage_isax_envelope_index<SaxEnvelope>);
            break;
        }
        case ISAX_ENV_W_ENV: {
            CONSTRUCT_ENVELOPE_INDEX(get_two_stage_isax_envelope_index<Envelope>);
            break;
        }
        case ISAX: {
            CONSTRUCT_PAA_INDEX(get_isax_index<Paa>);
            break;
        }
        case SAX_ENVELOPE: {
            CONSTRUCT_ENVELOPE_INDEX(get_flat_envelope_index<SaxEnvelope>);
            break;
        }
        case ENVELOPE: {
            CONSTRUCT_ENVELOPE_INDEX(get_flat_envelope_index<Envelope>);
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
