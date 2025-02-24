#include <filesystem>
#include <fstream>

#include "Modules/Indexing.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Index.hpp"
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

uptr<IiSaxSplitStrategy> get_split_strategy(const iSaxIndexParams *params, SaxSegIndT num_seg_per_channel,
                                            MtsNumChannelsT num_channels) {
    switch (params->split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<DoubleRoundRobinStrategy>(num_seg_per_channel, num_channels);
        case ENTROPY_MAXIMIZING:
            return std::make_unique<EntropyMaximizingStrategy>(params->min_num_bits_on_tie);
    }
    return nullptr;
}

uptr<IIndex<Envelope>> get_index(const IndexOptions &opts) {
    SearchMethodType search_method_type = opts.index_params->get_type();

    if (search_method_type == ISAX_ENVELOPE) {
        auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
        SaxSegIndT num_seg_per_channel = opts.l_max / params->segment_len;

        auto breakpoint_strategy = get_breakpoint_strategy(params);
        auto split_strategy = get_split_strategy(params, num_seg_per_channel, opts.num_channels);

        SeriesISaxProperties series_isax_prop = {
            params->segment_len, opts.series_len, params->pos_per_env, opts.num_channels, num_seg_per_channel,
        };

        SaxNumBitsT breakpoint_num_bits = DEFAULT_NUM_BIT_LIMIT;
        RunSettings::get_instance().set_isax_properties({num_seg_per_channel, params->segment_len,
                                                         breakpoint_strategy->get_breakpoints(1 << breakpoint_num_bits),
                                                         breakpoint_num_bits});

        auto *index = new iSaxIndex(series_isax_prop, params->first_layer_num_bits, params->leaf_capacity,
                                    std::move(split_strategy));
        return uptr<IIndex<Envelope>>(index);
    }
    return nullptr;
}

uptr<IEntryGenerator<Envelope>> get_envelope_generator(const IndexOptions &opts) {
    SearchMethodType search_method_type = opts.index_params->get_type();

    if (search_method_type == ISAX_ENVELOPE) {
        auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
        UlisseEnvelopeParams uli_params = {
            .pos_per_env = params->pos_per_env,
            .segment_len = params->segment_len,
            .l_min = opts.l_min,
            .l_max = opts.l_max,
        };
        return std::make_unique<iSaxEnvelopeGenerator>(opts.num_channels, opts.normalized, uli_params);
    }
    return nullptr;
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

    auto index = get_index(opts);
    if (std::ranges::find(ENVELOPE_METHODS, opts.index_params->get_type()) != ENVELOPE_METHODS.end()) {
        auto envelope_generator = get_envelope_generator(opts);
        logger.start_timer(ISC::INDEXING_TIME_S);
        index->construct(dataset_path, envelope_generator.get(), opts.num_channels, opts.series_len);
        std::ofstream index_stream(index_path, std::ios::binary);
        index->finalize()->save(index_stream, opts.index_format);
        logger.stop_timer(ISC::INDEXING_TIME_S);
    }

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
    }

    logger.write_entry();

    return 0;
}
