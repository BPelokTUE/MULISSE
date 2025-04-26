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
#include "Serialization/SerializationRegistration.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logging/IndexLogger.hpp"

uptr<IiSaxBreakpointStrategy> get_breakpoint_strategy(const SaxIndexParams *params) {
    switch (params->m_breakpoint_strategy_type) {
        case EQUIPROBABLE:
            return std::make_unique<EquiprobableBreakpointStrategy>();
        case FIXED:
            return std::make_unique<FixedBreakpointStrategy>(params->m_breakpoints_file);
    }
    return nullptr;
}

template <typename T>
uptr<IiSaxSplitStrategy<T>> get_split_strategy(const iSaxIndexParams *params, MtsNumChannelsT num_channels) {
    switch (params->m_split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<DoubleRoundRobinStrategy<T>>(params->m_num_segments, num_channels);
        case ENTROPY_MAXIMIZING:
            return std::make_unique<EntropyMaximizingStrategy<T>>(params->m_min_num_bits_on_tie);
        case ULISSE_CLOSEST_TO_MEAN:
            return std::make_unique<UlisseClosestToMeanStrategy<T>>();
        case CLOSES_TO_MEAN:
            return std::make_unique<ClosestToMeanStrategy<T>>();
    }
    return nullptr;
}

sptr<ISegmentationStrategy> get_segmentation_strategy(const PaaIndexParams *params, uint l_max) {
    switch (params->m_segmentation_strategy_type) {
        case UNIFORM:
            return std::make_shared<UniformSegmentationStrategy>(l_max, params->m_num_segments);
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
    auto *index =
        new iSaxPaaIndex(params->m_num_bits, params->m_leaf_capacity, segmentation_strategy, std::move(split_strategy));
    return sptr<IIndex<Paa>>(index);
}

template <>
sptr<IIndex<Envelope>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                      sptr<ISegmentationStrategy> segmentation_strategy,
                                      uptr<IiSaxSplitStrategy<Envelope>> split_strategy) {
    auto *env_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.m_index_params.get());
    auto *index = new iSaxEnvelopeIndex(env_params->m_num_bits, env_params->m_leaf_capacity, segmentation_strategy,
                                        std::move(split_strategy), env_params->m_pos_per_env);
    return sptr<IIndex<Envelope>>(index);
}

void calculate_sax_breakpoints(const SaxIndexParams *params, SaxNumBitsT num_bits_limit) {
    auto breakpoint_strategy = get_breakpoint_strategy(params);
    auto breakpoints = breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << num_bits_limit));
    RunSettings::get_instance().set_breakpoint_props({num_bits_limit, std::move(breakpoint_strategy), breakpoints});
}

// Index factory functions

struct IndexFactoryParams {
    bool m_discretize_flat_index = false;
    sptr<ISegmentationStrategy> m_segmentation_strategy;
    const IndexOptions &m_opts;
};

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(const IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());

    calculate_sax_breakpoints(params, params->m_num_bits_limit);
    auto split_strategy = get_split_strategy<T>(params, opts.m_num_channels);

    return get_isax_index<T>(opts, params, factory_params.m_segmentation_strategy, std::move(split_strategy));
}

sptr<IIndex<Envelope>> get_envelope_index(const IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;
    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());

    auto sax_params = dynamic_cast<SaxIndexParams *>(opts.m_index_params.get());
    if (discretize_flat_index && sax_params && sax_params->m_num_bits > 0) {
        calculate_sax_breakpoints(sax_params, sax_params->m_num_bits);
        auto *index = new FlatEnvelopeIndex(factory_params.m_segmentation_strategy, params->m_pos_per_env,
                                            sax_params->m_num_bits);
        return sptr<IIndex<Envelope>>(index);
    } else {
        auto *index = new FlatEnvelopeIndex(factory_params.m_segmentation_strategy, params->m_pos_per_env);
        return sptr<IIndex<Envelope>>(index);
    }
}

sptr<IIndex<Envelope>> get_envelope_tree_index(const IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *params = dynamic_cast<TreeEnvelopeIndexParams *>(opts.m_index_params.get());

    calculate_sax_breakpoints(params, params->m_num_bits);

    auto grouper = new InvSaxSortingBucketingEnvelopeGrouper(params->m_num_bits, params->m_bucket_size);
    auto *index = new TreeEnvelopeIndex(factory_params.m_segmentation_strategy, params->m_pos_per_env,
                                        uptr<IEnvelopeGrouper>(grouper));
    return sptr<IIndex<Envelope>>(index);
}

sptr<IIndex<Envelope>> get_two_stage_isax_envelope_index(const IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;

    vec<sptr<IIndex<Envelope>>> approx_indexes(1);
    approx_indexes[0] = get_isax_index<Envelope>(factory_params);
    if (discretize_flat_index) {
        auto isax_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.m_index_params.get());
        isax_params->m_num_bits = isax_params->m_num_bits_limit;
    }
    auto exact_index = get_envelope_index(factory_params);

    auto *index = new ChainIndex<Envelope>(std::move(approx_indexes), std::move(exact_index));
    return sptr<IIndex<Envelope>>(index);
}

// Generator getters

uptr<IEntryGenerator<Paa>> get_paa_generator(const IndexOptions &opts,
                                             const ISegmentationStrategy *segmentation_strategy) {
    iSaxPaaParams paa_params = {
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
        .m_segmentation_strategy = segmentation_strategy,
    };
    uint num_len_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;

    return std::make_unique<PaaEntryGenerator>(opts.m_num_channels, paa_params, num_len_groups);
}

uptr<IEntryGenerator<Envelope>> get_envelope_generator(const IndexOptions &opts,
                                                       const ISegmentationStrategy *segmentation_strategy) {
    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
    EnvelopeParams env_params = {
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
        .m_pos_per_env = params->m_pos_per_env,
        .m_segmentation_strategy = segmentation_strategy,
    };
    uint num_len_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;

    return std::make_unique<EnvelopeEntryGenerator>(opts.m_num_channels, opts.m_normalized, env_params, num_len_groups);
}

template <typename T>
    requires DerivedFromEntryData<T>
void construct_index(std::function<sptr<IIndex<T>>(const IndexFactoryParams &)> index_factory,
                     uptr<IEntryGenerator<T>> generator, IndexFactoryParams &factory_params) {
    auto &RS = RunSettings::get_instance();
    auto &logger = IndexLogger::get_instance();
    auto &opts = factory_params.m_opts;

    sptr<IIndex<T>> index;
    if (opts.m_l_per_group > 0) {
        uint num_len_groups = RS.get_length_props().m_num_l_groups;
        vec<sptr<IIndex<T>>> group_indexes(num_len_groups);
        for (uint lg_ind = 0; lg_ind < num_len_groups; lg_ind++) {
            group_indexes[lg_ind] = index_factory(factory_params);
        }
        index = std::make_shared<LengthGroupingIndex<T>>(std::move(group_indexes), opts.m_l_min, opts.m_l_max);
    } else {
        index = index_factory(factory_params);
    }

    logger.start_timer(ISC::INDEXING_TIME_S);
    index->construct(RS.get_dataset_path(), std::move(generator), opts.m_inserter_type, opts.m_num_channels,
                     opts.m_series_len, opts.m_adapt);
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

#define CONSTRUCT_INDEX(Type, index_factory, generator)                         \
    construct_index<Type>(                                                      \
        [](const IndexFactoryParams &factory_params_in) -> sptr<IIndex<Type>> { \
            return index_factory(factory_params_in);                            \
        },                                                                      \
        generator(opts, factory_params.m_segmentation_strategy.get()), factory_params);

    IndexFactoryParams factory_params{
        .m_discretize_flat_index = false,
        .m_segmentation_strategy =
            get_segmentation_strategy(dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get()), opts.m_l_max),
        .m_opts = opts,
    };

    switch (opts.m_index_method) {
        case ISAX_ENVELOPE:
            CONSTRUCT_INDEX(Envelope, get_isax_index<Envelope>, get_envelope_generator);
            break;
        case ISAX_ENV_W_SAX_ENV:
            factory_params.m_discretize_flat_index = true;
        case ISAX_ENV_W_ENV:
            CONSTRUCT_INDEX(Envelope, get_two_stage_isax_envelope_index, get_envelope_generator);
            break;
        case ISAX:
            CONSTRUCT_INDEX(Paa, get_isax_index<Paa>, get_paa_generator);
            break;
        case SAX_ENVELOPE:
            factory_params.m_discretize_flat_index = true;
        case ENVELOPE:
            CONSTRUCT_INDEX(Envelope, get_envelope_index, get_envelope_generator);
            break;
        case TREE_ENVELOPE:
            CONSTRUCT_INDEX(Envelope, get_envelope_tree_index, get_envelope_generator);
            break;
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
