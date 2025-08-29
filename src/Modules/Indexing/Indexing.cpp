#include "Modules/Indexing/Indexing.hpp"

#include "Util/Artefacts/IndexManager.hpp"
#include "Util/Logging/IndexLogger.hpp"

void create_index(IndexManager &index_manager, IndexGenOptions &index_gen_opts, IndexLogger &logger) {
    logger.start_timer(ISC::INDEXING_TIME_S);

    // Initialized SAX breakpoints
    index_manager.initialize_sax_breakpoints();

    // Set num_segments dynamically if required
    index_manager.set_optimal_params();

    // Estimate parameters if required
    index_manager.estimate_params(index_gen_opts);

    // Set up segmentation strategies
    logger.start_timer(ISC::SEGMENTATION_SETUP_TIME_S);
    index_manager.setup_segmentation_strategies();  // TODO
    // vec<Real> channel_scores = get_channel_scores(opts);
    // auto lg_segmentation_strategy = get_lg_segmentation_strategy(opts, &channel_scores);
    // logger.set_num_segments_cols(lg_segmentation_strategy.get(), log_num_seg_per_ch, log_num_seg_all);
    logger.stop_timer(ISC::SEGMENTATION_SETUP_TIME_S);

    // Estimate gamma if required
    index_manager.estimate_gamma(index_gen_opts);

    // Construct index
    index_manager.construct_index(logger);

    // #define CONSTRUCT_INDEX(Type, index_factory)                                                                          \
//     construct_index<Type>(                                                                                            \
//         [](IndexFactoryParams &factory_params_in) -> sptr<IIndex<Type>> { return index_factory(factory_params_in); }, \
//         std::move(generator), std::move(merger), opts, std::move(lg_segmentation_strategy), index_sample_frac);

    // #define CONSTRUCT_ENVELOPE_INDEX(index_factory)                                    \
//     auto generator = get_envelope_generator(opts, lg_segmentation_strategy.get()); \
//     auto merger = get_entry_merger<Envelope>(opts);                                \
//     CONSTRUCT_INDEX(Envelope, index_factory);

    // #define CONSTRUCT_PAA_INDEX(index_factory)                                    \
//     auto generator = get_paa_generator(opts, lg_segmentation_strategy.get()); \
//     auto merger = get_entry_merger<Paa>(opts);                                \
//     CONSTRUCT_INDEX(Paa, index_factory);

    //     switch (opts.m_index_method) {
    //         case ISAX_ENVELOPE: {
    //             CONSTRUCT_ENVELOPE_INDEX(get_isax_index<Envelope>);
    //             break;
    //         }
    //         case ISAX_ENV_W_SAX_ENV: {
    //             CONSTRUCT_ENVELOPE_INDEX(get_two_stage_isax_envelope_index<SaxEnvelope>);
    //             break;
    //         }
    //         case ISAX_ENV_W_ENV: {
    //             CONSTRUCT_ENVELOPE_INDEX(get_two_stage_isax_envelope_index<Envelope>);
    //             break;
    //         }
    //         case ISAX: {
    //             CONSTRUCT_PAA_INDEX(get_isax_index<Paa>);
    //             break;
    //         }
    //         case SAX_ENVELOPE: {
    //             CONSTRUCT_ENVELOPE_INDEX(get_flat_envelope_index<SaxEnvelope>);
    //             break;
    //         }
    //         case ENVELOPE: {
    //             CONSTRUCT_ENVELOPE_INDEX(get_flat_envelope_index<Envelope>);
    //             break;
    //         }
    //         case TREE_ENVELOPE:
    //         case BUCKETING_ENVELOPE:
    //         case VL_ENVELOPE: {
    //             CONSTRUCT_ENVELOPE_INDEX(get_envelope_tree_index);
    //             break;
    //         }
    //         default:
    //             break;
    //     }
    logger.stop_timer(ISC::INDEXING_TIME_S);

    // if (RS.ffts_supported()) {
    //     logger.start_timer(ISC::FFT_CALC_TIME_S);
    //     RS.calculate_ffts();
    //     logger.stop_timer(ISC::FFT_CALC_TIME_S);
    //     logger.increment_count_col(ISC::SIZE_ON_DISK_B, RS.get_ffts_size_on_disk());
    // }

    logger.write_entry();
}
