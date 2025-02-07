#include <filesystem>
#include <fstream>

#include "Modules/Indexing.hpp"
#include "Search/EnvelopeIndex.hpp"
#include "Search/iSax/iSaxEnvelopeIndex.hpp"
#include "Util/RunSettings.hpp"

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
            return std::make_unique<EntropyMaximizingStrategy>(false);
    }
    return nullptr;
}

uptr<IEnvelopeIndex> get_index(const IndexOptions &opts) {
    switch (opts.index_params->get_type()) {
        case ISAX_ENVELOPE:
            auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
            SaxSegIndT num_seg_per_channel = opts.l_max / params->segment_len;

            auto breakpoint_strategy = get_breakpoint_strategy(params);
            auto split_strategy = get_split_strategy(params, num_seg_per_channel, opts.num_channels);

            SeriesISaxProperties series_isax_prop = {
                params->segment_len, opts.series_len, params->pos_per_env, opts.num_channels, num_seg_per_channel,
            };

            SaxNumBitsT breakpoint_num_bits = DEFAULT_NUM_BIT_LIMIT;
            RunSettings::get_instance().set_isax_properties(
                {num_seg_per_channel, params->segment_len,
                 breakpoint_strategy->get_breakpoints(1 << breakpoint_num_bits), breakpoint_num_bits});

            auto *index = new iSaxEnvelopeIndex(series_isax_prop, params->first_layer_num_bits, params->leaf_capacity,
                                                std::move(split_strategy));
            return uptr<IEnvelopeIndex>(index);
    }
    return nullptr;
}

uptr<IEnvelopeGenerator> get_envelope_generator(const IndexOptions &opts) {
    switch (opts.index_params->get_type()) {
        case ISAX_ENVELOPE:
            auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
            UlisseEnvelopeParams uli_params = {
                .pos_per_env = params->pos_per_env,
                .segment_len = params->segment_len,
                .l_min = opts.l_min,
                .l_max = opts.l_max,
            };
            return std::make_unique<iSaxEnvelopeGenerator>(opts.num_channels, opts.normalized, uli_params);
    }
}

int create_index(const IndexOptions &opts) {
    if (!std::filesystem::exists(opts.dataset_path)) {
        std::cerr << "Error: Dataset " << opts.dataset_path << " does not exist." << std::endl;
        return 1;
    }

    auto index = get_index(opts);

    if (std::ranges::find(ENVELOPE_TYPES, opts.index_params->get_type()) != ENVELOPE_TYPES.end()) {
        auto envelope_generator = get_envelope_generator(opts);
        index->construct(opts.dataset_path, envelope_generator.get(), opts.num_channels, opts.series_len);
        std::ofstream index_stream(opts.index_path, std::ios::binary);
        index->finalize()->save(index_stream, opts.index_format);
    }

    auto &run_settings = RunSettings::get_instance();
    if (run_settings.ffts_supported()) run_settings.calculate_ffts();

    return 0;
}
