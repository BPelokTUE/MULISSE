#include <filesystem>
#include <fstream>
#include <functional>

#include "Modules/Indexing.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Index.hpp"
#include "Search/Envelope/FlatEnvelopeIndex.hpp"
#include "Search/Envelope/TreeEnvelopeIndex.hpp"
#include "Search/Envelope/EnvelopeGrouper.hpp"
#include "Search/iSax/iSaxIndex.hpp"
#include "Search/ChainIndex.hpp"
#include "Search/LengthGroupingIndex.hpp"
#include "Search/TopDownInserter.hpp"
#include "Summarization/SegmentationStrategy.hpp"
#include "Summarization/LengthGroupSegmentationStrategy.hpp"
#include "Summarization/EnvelopeEntryMerger.hpp"
#include "Serialization/SerializationRegistration.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logging/IndexLogger.hpp"

// Strategies

uptr<ISaxBreakpointStrategy> get_breakpoint_strategy(const SaxParams &sax_params) {
    switch (sax_params.m_breakpoint_strategy_type) {
        case EQUIPROBABLE:
            return std::make_unique<EquiprobableBreakpointStrategy>();
        case FIXED:
            return std::make_unique<FixedBreakpointStrategy>(sax_params.m_breakpoints_file);
    }
    return nullptr;
}

template <typename T>
uptr<IiSaxSplitStrategy<T>> get_split_strategy(const iSaxIndexParams *index_params, MtsNumChannelsT num_channels) {
    switch (index_params->m_isax_trie_params.m_split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<DoubleRoundRobinStrategy<T>>(index_params->m_segmentation_params.m_num_segments,
                                                                 num_channels);
        case ENTROPY_MAXIMIZING:
            return std::make_unique<EntropyMaximizingStrategy<T>>(
                index_params->m_isax_trie_params.m_min_num_bits_on_tie);
        case ULISSE_CLOSEST_TO_MEAN:
            return std::make_unique<UlisseClosestToMeanStrategy<T>>();
        case CLOSEST_TO_MEAN:
            return std::make_unique<ClosestToMeanStrategy<T>>();
    }
    return nullptr;
}

sptr<ISegmentationStrategy> get_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                      SaxSegIndT num_segments) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    switch (index_params->m_segmentation_params.m_strategy_type) {
        case UNIFORM:
            return std::make_unique<UniformSegmentationStrategy>(l_max, num_segments);
        case ADAPTIVE:
            uint pos_per_env = 0;
            if (auto *env_params = dynamic_cast<const EnvelopeIndexParams *>(opts.m_index_params.get())) {
                pos_per_env = env_params->m_enveloping_params.m_pos_per_env;
            }
            return std::make_unique<AdaptiveSegmentationStrategy>(l_min, l_max, opts.m_series_len, num_segments,
                                                                  pos_per_env);
    }
    return nullptr;
}

uptr<ILengthGroupSegmentationStrategy> get_lg_segmentation_strategy(const IndexOptions &opts) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    switch (index_params->m_segmentation_params.m_lg_strategy_type) {
        case SINGLE:
            return std::make_unique<SingleSegmentationStrategy>(get_segmentation_strategy(
                opts, opts.m_l_min, opts.m_l_max, index_params->m_segmentation_params.m_num_segments));
        case MULTI:
            return std::make_unique<MultiSegmentationStrategy>([&opts, index_params](uint lg_l_min, uint lg_l_max) {
                return get_segmentation_strategy(opts, lg_l_min, lg_l_max,
                                                 index_params->m_segmentation_params.m_num_segments);
            });
        case ADAPTIVE_MULTI:
            uint pos_per_env = 0;
            if (auto *env_params = dynamic_cast<const EnvelopeIndexParams *>(opts.m_index_params.get())) {
                pos_per_env = env_params->m_enveloping_params.m_pos_per_env;
            }
            return std::make_unique<AdaptiveMultiSegmentationStrategy>(
                [&opts](uint lg_l_min, uint lg_l_max, SaxSegIndT num_segments) {
                    return get_segmentation_strategy(opts, lg_l_min, lg_l_max, num_segments);
                },
                index_params->m_segmentation_params.m_num_segments, pos_per_env);
    }
    return nullptr;
}

// Generators

uptr<IEntryGenerator<Paa>> get_paa_generator(const IndexOptions &opts,
                                             const ILengthGroupSegmentationStrategy *segmentation_strategies) {
    PaaParams paa_params = {
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
        .m_lg_segmentation_strategy = segmentation_strategies,
    };
    uint num_len_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;

    return std::make_unique<PaaEntryGenerator>(opts.m_num_channels, paa_params, num_len_groups);
}

uptr<IEntryGenerator<Envelope>> get_envelope_generator(
    const IndexOptions &opts, const ILengthGroupSegmentationStrategy *segmentation_strategies) {
    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
    EnvelopeParams env_params = {
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
        .m_pos_per_env = params->m_enveloping_params.m_pos_per_env,
        .m_segmentation_strategies = segmentation_strategies,
    };
    uint num_len_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;

    return std::make_unique<EnvelopeEntryGenerator>(opts.m_num_channels, opts.m_normalized, env_params, num_len_groups);
}

// Mergers

uptr<IEnvelopeEntryMerger> get_envelope_merger(const IndexOptions &opts) {
    auto &params = dynamic_cast<EnvelopeIndexParams &>(*opts.m_index_params);
    switch (params.m_enveloping_params.m_entry_merger_type) {
        case DUMMY:
            return std::make_unique<DummyEnvelopeEntryMerger>();
        case SAX_BASED:
            return std::make_unique<SaxBasedEnvelopeEntryMerger>(
                params.m_enveloping_params.m_merger_sax_params->m_num_bits);
        case LOWER_SAX_BASED:
            return std::make_unique<LowerSaxBasedEnvelopeEntryMerger>(
                params.m_enveloping_params.m_merger_sax_params->m_num_bits);
    }
    return nullptr;
}

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                               sptr<ISegmentationStrategy> segmentation_strategy,
                               uptr<IiSaxSplitStrategy<T>> split_strategy);

template <>
sptr<IIndex<Paa>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                 sptr<ISegmentationStrategy> segmentation_strategy,
                                 uptr<IiSaxSplitStrategy<Paa>> split_strategy) {
    auto *index = new iSaxPaaIndex(params->m_sax_params.m_num_bits, params->m_isax_trie_params.m_leaf_capacity,
                                   segmentation_strategy, std::move(split_strategy));
    return sptr<IIndex<Paa>>(index);
}

template <>
sptr<IIndex<Envelope>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                      sptr<ISegmentationStrategy> segmentation_strategy,
                                      uptr<IiSaxSplitStrategy<Envelope>> split_strategy) {
    auto *env_index_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.m_index_params.get());
    auto *index = new iSaxEnvelopeIndex(env_index_params->m_sax_params.m_num_bits,
                                        env_index_params->m_isax_trie_params.m_leaf_capacity, segmentation_strategy,
                                        std::move(split_strategy), env_index_params->m_enveloping_params.m_pos_per_env);
    return sptr<IIndex<Envelope>>(index);
}

// TODO: rewrite, pass breakpoints and number of bits to relevant classes directly
void initialize_sax_breakpoints(const SaxParams &sax_params, SaxNumBitsT num_bits_limit) {
    auto &RS = RunSettings::get_instance();
    if (!RS.breakpoints_set()) {
        auto breakpoint_strategy = get_breakpoint_strategy(sax_params);
        auto breakpoints = breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << num_bits_limit));
        RS.set_breakpoint_props({num_bits_limit, std::move(breakpoint_strategy), breakpoints});
    }
}

// Index factory functions

struct IndexFactoryParams {
    bool m_discretize_flat_index = false;
    sptr<ISegmentationStrategy> m_segmentation_strategy;
    const IndexOptions &m_opts;
};

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());
    auto split_strategy = get_split_strategy<T>(index_params, opts.m_num_channels);

    return get_isax_index<T>(opts, index_params, factory_params.m_segmentation_strategy, std::move(split_strategy));
}

sptr<IIndex<Envelope>> get_envelope_index(IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());

    auto sax_index_params = dynamic_cast<SaxIndexParams *>(opts.m_index_params.get());
    if (discretize_flat_index && sax_index_params && sax_index_params->m_sax_params.m_num_bits > 0) {
        auto *index = new FlatEnvelopeIndex(factory_params.m_segmentation_strategy,
                                            index_params->m_enveloping_params.m_pos_per_env,
                                            sax_index_params->m_sax_params.m_num_bits);
        return sptr<IIndex<Envelope>>(index);
    } else {
        auto *index = new FlatEnvelopeIndex(factory_params.m_segmentation_strategy,
                                            index_params->m_enveloping_params.m_pos_per_env);
        return sptr<IIndex<Envelope>>(index);
    }
}

sptr<IIndex<Envelope>> get_envelope_tree_index(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<TreeEnvelopeIndexParams *>(opts.m_index_params.get());

    auto grouper =
        new InvSaxSortingBucketingEnvelopeGrouper(index_params->m_sax_params.m_num_bits, index_params->m_bucket_size);
    auto *index =
        new TreeEnvelopeIndex(factory_params.m_segmentation_strategy, index_params->m_enveloping_params.m_pos_per_env,
                              uptr<IEnvelopeGrouper>(grouper));
    return sptr<IIndex<Envelope>>(index);
}

sptr<IIndex<Envelope>> get_two_stage_isax_envelope_index(IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;

    vec<sptr<IIndex<Envelope>>> approx_indexes(1);
    approx_indexes[0] = get_isax_index<Envelope>(factory_params);
    if (discretize_flat_index) {
        auto isax_index_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.m_index_params.get());
        isax_index_params->m_sax_params.m_num_bits = isax_index_params->m_isax_trie_params.m_num_bits_limit;
    }
    auto exact_index = get_envelope_index(factory_params);

    auto *index = new ChainIndex<Envelope>(std::move(approx_indexes), std::move(exact_index));
    return sptr<IIndex<Envelope>>(index);
}

template <typename T>
    requires DerivedFromEntryData<T>
void construct_index(std::function<sptr<IIndex<T>>(IndexFactoryParams &)> index_factory,
                     uptr<IEntryGenerator<T>> generator, uptr<IEnvelopeEntryMerger> envelope_merger,
                     IndexFactoryParams &factory_params,
                     uptr<ILengthGroupSegmentationStrategy> lg_segmentation_strategy) {
    auto &RS = RunSettings::get_instance();
    auto &logger = IndexLogger::get_instance();
    auto &opts = factory_params.m_opts;

    sptr<IIndex<T>> index;
    if (opts.m_use_length_groups) {
        uint num_len_groups = RS.get_length_props().m_num_l_groups;

        vec<sptr<IIndex<T>>> group_indexes(num_len_groups);
        for (uint lg_ind = 0; lg_ind < num_len_groups; lg_ind++) {
            IndexFactoryParams lg_factory_params{
                .m_discretize_flat_index = factory_params.m_discretize_flat_index,
                .m_segmentation_strategy = lg_segmentation_strategy->get_segmentation_strategy(lg_ind),
                .m_opts = opts,
            };
            group_indexes[lg_ind] = index_factory(lg_factory_params);
        }
        index = std::make_shared<LengthGroupingIndex<T>>(std::move(group_indexes), opts.m_l_min, opts.m_l_max);
    } else {
        factory_params.m_segmentation_strategy = lg_segmentation_strategy->get_segmentation_strategy(0);
        index = index_factory(factory_params);
    }

    logger.start_timer(ISC::INDEXING_TIME_S);
    index->construct(RS.get_dataset_path(), std::move(generator), std::move(envelope_merger), opts.m_inserter_type,
                     opts.m_num_channels, opts.m_series_len, opts.m_adapt);
    auto finalized_index = index->finalize();
    finalized_index->save(RS.get_index_path(), opts.m_index_format);
    logger.stop_timer(ISC::INDEXING_TIME_S);

    logger.increment_count_col(ISC::SIZE_ON_DISK_B, finalized_index->get_size_on_disk(RS.get_index_path()));
}

int create_index(const IndexOptions &opts) {
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();
    str index_path = RS.get_index_path();

    if (!std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " does not exist." << std::endl;
        return 1;
    }

    IndexLogger::initialize(opts);
    auto &logger = IndexLogger::get_instance();

    // Initialized SAX breakpoints

    if (arr_contains(METHODS_W_ISAX, opts.m_index_method)) {
        auto *index_params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());
        if (index_params && index_params->m_isax_trie_params.m_num_bits_limit > 0) {
            initialize_sax_breakpoints(index_params->m_sax_params, index_params->m_isax_trie_params.m_num_bits_limit);
        } else {
            throw std::runtime_error("iSAX index method requires a positive number of bits limit");
        }
    } else if (arr_contains(METHODS_W_SAX, opts.m_index_method)) {
        auto *index_params = dynamic_cast<SaxIndexParams *>(opts.m_index_params.get());
        if (index_params && index_params->m_sax_params.m_num_bits > 0) {
            initialize_sax_breakpoints(index_params->m_sax_params, index_params->m_sax_params.m_num_bits);
        } else {
            throw std::runtime_error("SAX index method requires a positive number of bits");
        }
    } else if (arr_contains(METHODS_W_ENVELOPE, opts.m_index_method)) {
        auto *index_params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
        if (index_params && arr_contains(MERGERS_W_SAX, index_params->m_enveloping_params.m_entry_merger_type)) {
            auto merger_sax_params = index_params->m_enveloping_params.m_merger_sax_params;
            if (merger_sax_params && merger_sax_params->m_num_bits > 0) {
                initialize_sax_breakpoints(*merger_sax_params, merger_sax_params->m_num_bits);
            } else {
                throw std::runtime_error("SAX-based envelope entry merger requires a positive number of bits");
            }
        }
    }

    // Create index

#define CONSTRUCT_INDEX(Type, index_factory)                                                                          \
    construct_index<Type>(                                                                                            \
        [](IndexFactoryParams &factory_params_in) -> sptr<IIndex<Type>> { return index_factory(factory_params_in); }, \
        std::move(generator), std::move(envelope_merger), factory_params, std::move(lg_segmentation_strategy));

#define CONSTRUCT_ENVELOPE_INDEX(index_factory)                                    \
    auto generator = get_envelope_generator(opts, lg_segmentation_strategy.get()); \
    envelope_merger = get_envelope_merger(opts);                                   \
    CONSTRUCT_INDEX(Envelope, index_factory);

#define CONSTRUCT_PAA_INDEX(index_factory)                                    \
    auto generator = get_paa_generator(opts, lg_segmentation_strategy.get()); \
    CONSTRUCT_INDEX(Paa, index_factory);

    auto lg_segmentation_strategy = get_lg_segmentation_strategy(opts);
    IndexFactoryParams factory_params{
        .m_discretize_flat_index = false,
        .m_opts = opts,
    };

    uptr<IEnvelopeEntryMerger> envelope_merger = nullptr;

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
            CONSTRUCT_ENVELOPE_INDEX(get_envelope_index);
            break;
        }
        case TREE_ENVELOPE: {
            CONSTRUCT_ENVELOPE_INDEX(get_envelope_tree_index);
            break;
        }
        default:
            break;
    }

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
        logger.increment_count_col(ISC::SIZE_ON_DISK_B, RS.get_ffts_size_on_disk());
    }

    logger.write_entry();

    return 0;
}
