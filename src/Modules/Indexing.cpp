#include <filesystem>
#include <fstream>

#include "Modules/Indexing.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Index.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/iSax/iSaxIndex.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"

uptr<IiSaxBreakpointStrategy> get_breakpoint_strategy(const iSaxIndexParams *params) {
    switch (params->breakpoint_strategy_type) {
        case EQUIPROBABLE:
            return std::make_unique<EquiprobableBreakpointStrategy>();
    }
    return nullptr;
}

template <typename T>
uptr<IiSaxSplitStrategy<T>> get_split_strategy(const iSaxIndexParams *params, SaxSegIndT num_seg_per_channel,
                                               MtsNumChannelsT num_channels) {
    switch (params->split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<DoubleRoundRobinStrategy<T>>(num_seg_per_channel, num_channels);
        case ENTROPY_MAXIMIZING:
            return std::make_unique<EntropyMaximizingStrategy<T>>(params->min_num_bits_on_tie);
    }
    return nullptr;
}

template <typename T>
    requires DerivedFromEntryData<T>
uptr<IIndex<T>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params, SaxSegIndT num_seg_per_channel,
                               uptr<IiSaxSplitStrategy<T>> split_strategy);

template <>
uptr<IIndex<Paa>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                 SaxSegIndT num_seg_per_channel, uptr<IiSaxSplitStrategy<Paa>> split_strategy) {
    auto series_isax_prop = std::make_unique<SeriesISaxProperties>(params->segment_len, opts.series_len,
                                                                   opts.num_channels, num_seg_per_channel);
    auto *index = new iSaxPaaIndex(std::move(series_isax_prop), params->first_layer_num_bits, params->leaf_capacity,
                                   std::move(split_strategy));
    return uptr<IIndex<Paa>>(index);
}

template <>
uptr<IIndex<Envelope>> get_isax_index(const IndexOptions &opts, const iSaxIndexParams *params,
                                      SaxSegIndT num_seg_per_channel,
                                      uptr<IiSaxSplitStrategy<Envelope>> split_strategy) {
    auto *env_params = dynamic_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
    auto series_isax_prop = std::make_unique<SeriesISaxEnvelopeProperties>(
        env_params->segment_len, opts.series_len, opts.num_channels, num_seg_per_channel, env_params->pos_per_env);

    auto *index = new iSaxEnvelopeIndex(std::move(series_isax_prop), env_params->first_layer_num_bits,
                                        env_params->leaf_capacity, std::move(split_strategy));
    return uptr<IIndex<Envelope>>(index);
}

template <typename T>
    requires DerivedFromEntryData<T>
uptr<IIndex<T>> get_isax_index(const IndexOptions &opts) {
    auto *params = dynamic_cast<iSaxIndexParams *>(opts.index_params.get());
    SaxSegIndT num_seg_per_channel = opts.l_max / params->segment_len;

    auto breakpoint_strategy = get_breakpoint_strategy(params);
    auto split_strategy = get_split_strategy<T>(params, num_seg_per_channel, opts.num_channels);

    SaxNumBitsT breakpoint_num_bits = DEFAULT_NUM_BIT_LIMIT;
    auto breakpoints = breakpoint_strategy->get_breakpoints(1 << breakpoint_num_bits);
    RunSettings::get_instance().set_isax_properties(
        {num_seg_per_channel, params->segment_len, std::move(breakpoint_strategy), breakpoints, breakpoint_num_bits});

    return get_isax_index<T>(opts, params, num_seg_per_channel, std::move(split_strategy));
}

uptr<IIndex<Envelope>> get_envelope_index(const IndexOptions &opts) {
    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.index_params.get());
    SaxSegIndT num_seg_per_channel = opts.l_max / params->segment_len;
    auto *index = new FlatEnvelopeIndex(params->segment_len, params->pos_per_env);
    return uptr<IIndex<Envelope>>(index);
}

uptr<IEntryGenerator<Paa>> get_paa_generator(const IndexOptions &opts) {
    auto *params = dynamic_cast<iSaxIndexParams *>(opts.index_params.get());
    iSaxPaaParams paa_params = {
        .segment_len = params->segment_len,
        .l_min = opts.l_min,
        .l_max = opts.l_max,
    };
    return std::make_unique<PaaEntryGenerator>(opts.num_channels, paa_params);
}

uptr<IEntryGenerator<Envelope>> get_envelope_generator(const IndexOptions &opts) {
    auto *params = dynamic_cast<EnvelopeIndexParams *>(opts.index_params.get());
    UlisseEnvelopeParams uli_params = {
        .pos_per_env = params->pos_per_env,
        .segment_len = params->segment_len,
        .l_min = opts.l_min,
        .l_max = opts.l_max,
    };
    return std::make_unique<EnvelopeEntryGenerator>(opts.num_channels, opts.normalized, uli_params);
}

template <typename T>
    requires DerivedFromEntryData<T>
void construct_index(uptr<IIndex<T>> index, uptr<IEntryGenerator<T>> generator, const IndexOptions &opts,
                     RunSettings &RS, IndexLogger &logger) {
    logger.start_timer(ISC::INDEXING_TIME_S);
    index->construct(RS.get_dataset_path(), generator.get(), opts.num_channels, opts.series_len, opts.adapt);
    std::ofstream index_stream(RS.get_index_path(), std::ios::binary);
    index->finalize()->save(index_stream, opts.index_format);
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

    if (opts.index_params->get_type() == ISAX_ENVELOPE) {
        construct_index(get_isax_index<Envelope>(opts), get_envelope_generator(opts), opts, RS, logger);
    } else if (opts.index_params->get_type() == ISAX) {
        construct_index(get_isax_index<Paa>(opts), get_paa_generator(opts), opts, RS, logger);
    } else {  // ENVELOPE
        construct_index(get_envelope_index(opts), get_envelope_generator(opts), opts, RS, logger);
    }

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
    }

    logger.write_entry();

    return 0;
}
