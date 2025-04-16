#include <filesystem>
#include <fstream>
#include <functional>

#include "Modules/Indexing.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Index.hpp"
#include "Search/Envelope/FlatEnvelopeIndex.hpp"
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
uptr<IiSaxSplitStrategy<T>> get_split_strategy(const iSaxIndexParams *params, SaxSegIndT num_seg_per_channel,
                                               MtsNumChannelsT num_channels) {
    switch (params->m_split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<DoubleRoundRobinStrategy<T>>(num_seg_per_channel, num_channels);
        case ENTROPY_MAXIMIZING:
            return std::make_unique<EntropyMaximizingStrategy<T>>(params->m_min_num_bits_on_tie);
        case ULISSE_CLOSEST_TO_MEAN:
            return std::make_unique<UlisseClosestToMeanStrategy<T>>();
        case CLOSES_TO_MEAN:
            return std::make_unique<ClosestToMeanStrategy<T>>();
    }
    return nullptr;
}

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params, SaxSegIndT num_seg_per_channel,
                               uptr<IiSaxSplitStrategy<T>> split_strategy);

template <>
sptr<IIndex<Paa>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                 SaxSegIndT num_seg_per_channel, uptr<IiSaxSplitStrategy<Paa>> split_strategy) {
    auto series_isax_prop = std::make_unique<SeriesISaxProperties>(params->m_segment_len, opts.m_series_len,
                                                                   opts.m_num_channels, num_seg_per_channel);
    auto *index = new iSaxPaaIndex(std::move(series_isax_prop), params->m_num_bits, params->m_leaf_capacity,
                                   std::move(split_strategy));
    return sptr<IIndex<Paa>>(index);
}

template <>
sptr<IIndex<Envelope>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                      SaxSegIndT num_seg_per_channel,
                                      uptr<IiSaxSplitStrategy<Envelope>> split_strategy) {
    auto *env_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.m_index_params.get());
    auto series_isax_prop = std::make_unique<SeriesISaxEnvelopeProperties>(env_params->m_segment_len, opts.m_series_len,
                                                                           opts.m_num_channels, num_seg_per_channel,
                                                                           env_params->m_pos_per_env);

    auto *index = new iSaxEnvelopeIndex(std::move(series_isax_prop), env_params->m_num_bits,
                                        env_params->m_leaf_capacity, std::move(split_strategy));
    return sptr<IIndex<Envelope>>(index);
}

void calculate_sax_breakpoints(const SaxIndexParams *params, SaxNumBitsT num_bits_limit,
                               SaxSegIndT num_seg_per_channel) {
    auto breakpoint_strategy = get_breakpoint_strategy(params);
    auto breakpoints = breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << num_bits_limit));
    RunSettings::get_instance().set_isax_properties(
        {num_seg_per_channel, params->m_segment_len, std::move(breakpoint_strategy), breakpoints, num_bits_limit});
}

// Index factory functions

struct IndexFactoryParams {
    bool m_discretize_flat_index = false;
    uint m_l_max;
    const IndexOptions &m_opts;
};

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(const IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;

    auto *params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());
    SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(factory_params.m_l_max / params->m_segment_len);

    calculate_sax_breakpoints(params, params->m_num_bits_limit, num_seg_per_channel);
    auto split_strategy = get_split_strategy<T>(params, num_seg_per_channel, opts.m_num_channels);

    return get_isax_index<T>(opts, params, num_seg_per_channel, std::move(split_strategy));
}

sptr<IIndex<Envelope>> get_envelope_index(const IndexFactoryParams &factory_params) {
    bool discretize_flat_index = factory_params.m_discretize_flat_index;
    const IndexOptions &opts = factory_params.m_opts;

    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
    SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(factory_params.m_l_max / params->m_segment_len);

    auto sax_params = dynamic_cast<SaxIndexParams *>(opts.m_index_params.get());
    if (discretize_flat_index && sax_params && sax_params->m_num_bits > 0) {
        calculate_sax_breakpoints(sax_params, sax_params->m_num_bits, num_seg_per_channel);
        auto *index = new FlatEnvelopeIndex(params->m_segment_len, params->m_pos_per_env, sax_params->m_num_bits);
        return sptr<IIndex<Envelope>>(index);
    } else {
        auto *index = new FlatEnvelopeIndex(params->m_segment_len, params->m_pos_per_env);
        return sptr<IIndex<Envelope>>(index);
    }
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

uptr<IEntryGenerator<Paa>> get_paa_generator(const IndexOptions &opts) {
    auto *params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());
    iSaxPaaParams paa_params = {
        .m_segment_len = params->m_segment_len,
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
    };
    uint num_len_groups = opts.get_num_len_groups();

    return std::make_unique<PaaEntryGenerator>(opts.m_num_channels, paa_params, num_len_groups);
}

uptr<IEntryGenerator<Envelope>> get_envelope_generator(const IndexOptions &opts) {
    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.m_index_params.get());
    EnvelopeParams env_params = {
        .m_pos_per_env = params->m_pos_per_env,
        .m_segment_len = params->m_segment_len,
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
    };
    uint num_len_groups = opts.get_num_len_groups();

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
        uint num_len_groups = opts.get_num_len_groups();
        vec<sptr<IIndex<T>>> group_indexes(num_len_groups);
        for (uint lg_ind = 0; lg_ind < num_len_groups; lg_ind++) {
            factory_params.m_l_max = opts.m_l_min + (opts.m_l_max - opts.m_l_min) * (lg_ind + 1) / num_len_groups;
            group_indexes[lg_ind] = index_factory(factory_params);
        }
        index = std::make_shared<LengthGroupingIndex<T>>(std::move(group_indexes), opts.m_l_min, opts.m_l_max);
    } else {
        factory_params.m_l_max = opts.m_l_max;
        index = index_factory(factory_params);
    }

    logger.start_timer(ISC::INDEXING_TIME_S);
    index->construct(RS.get_dataset_path(), std::move(generator), opts.m_inserter_type, opts.m_num_channels,
                     opts.m_series_len, opts.m_adapt);
    index->finalize()->save(RS.get_index_path(), opts.m_index_format);
    logger.stop_timer(ISC::INDEXING_TIME_S);
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
        generator, factory_params);

    IndexFactoryParams factory_params{
        .m_discretize_flat_index = false,
        .m_opts = opts,
    };

    switch (opts.m_index_method) {
        case ISAX_ENVELOPE:
            CONSTRUCT_INDEX(Envelope, get_isax_index<Envelope>, get_envelope_generator(opts));
            break;
        case ISAX_ENV_W_SAX_ENV:
            factory_params.m_discretize_flat_index = true;
        case ISAX_ENV_W_ENV:
            CONSTRUCT_INDEX(Envelope, get_two_stage_isax_envelope_index, get_envelope_generator(opts));
            break;
        case ISAX:
            CONSTRUCT_INDEX(Paa, get_isax_index<Paa>, get_paa_generator(opts));
            break;
        case SAX_ENVELOPE:
            factory_params.m_discretize_flat_index = true;
        case ENVELOPE:
            CONSTRUCT_INDEX(Envelope, get_envelope_index, get_envelope_generator(opts));
            break;
        default:
            break;
    }

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
    }

    logger.write_entry();

    return 0;
}
