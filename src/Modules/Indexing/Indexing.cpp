#include <filesystem>
#include <fstream>
#include <iostream>

#include "Modules/Indexing/Indexing.hpp"
#include "Modules/Indexing/EnvelopeGenerator.hpp"
#include "Search/IEnvelopeIndex.hpp"
#include "Search/iSax/iSaxEnvelopeIndex.hpp"

std::unique_ptr<IiSaxBreakpointStrategy> get_breakpoint_strategy(const iSaxIndexParams *params) {
    switch (params->breakpoint_strategy_type) {
        case EQUIPROBABLE:
            return std::make_unique<EquiprobableBreakpointStrategy>();
    }
    return nullptr;
}

std::unique_ptr<IiSaxSplitStrategy> get_split_strategy(const iSaxIndexParams *params, SaxSegIndT num_seg_per_channel,
                                                       MtsNumChannelsT num_channels) {
    switch (params->split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<DoubleRoundRobinStrategy>(num_seg_per_channel, num_channels);
    }
    return nullptr;
}

std::unique_ptr<IEnvelopeIndex> get_index(const IndexOptions &opts) {
    switch (opts.index_params->get_type()) {
        case ISAX_ENVELOPE:
            auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
            SaxSegIndT num_seg_per_channel = opts.l_max / params->segment_len;

            auto breakpoint_strategy = get_breakpoint_strategy(params);
            auto split_strategy = get_split_strategy(params, num_seg_per_channel, opts.num_channels);

            auto *index = new iSaxEnvelopeIndex(num_seg_per_channel, opts.num_channels, params->first_layer_num_bits,
                                                params->leaf_capacity, std::move(breakpoint_strategy),
                                                std::move(split_strategy), params->num_bits_limit);
            return std::unique_ptr<IEnvelopeIndex>(index);
    }
    return nullptr;
}

std::unique_ptr<IEnvelopeGenerator> get_envelope_generator(const IndexOptions &opts) {
    switch (opts.index_params->get_type()) {
        case ISAX_ENVELOPE:
            return std::make_unique<iSaxEnvelopeGenerator>(opts);
    }
}

int create_index(const IndexOptions &opts) {
    if (!std::filesystem::exists(opts.dataset_path)) {
        std::cerr << "Error: Dataset " << opts.dataset_path << " does not exist." << std::endl;
        return 1;
    }

    std::ifstream data_stream(opts.dataset_path, std::ios::binary);

    unsigned N = get_dataset_size(opts.dataset_path);
    unsigned num_series = N / (opts.num_channels * opts.series_len * sizeof(float));

    auto index = get_index(opts);

    if (std::ranges::find(ENVELOPE_TYPES, opts.index_params->get_type()) != ENVELOPE_TYPES.end()) {
        auto envelope_generator = get_envelope_generator(opts);

#pragma omp parallel for
        for (size_t i = 0; i < num_series; ++i) {
            vec<vec<float>> mts(opts.num_channels, vec<float>(opts.series_len));
            for (MtsNumChannelsT c = 0; c < opts.num_channels; ++c) {
                data_stream.read(reinterpret_cast<char *>(mts[c].data()), opts.series_len * sizeof(float));
            }

            auto entries = envelope_generator->get_entries(mts);
#pragma omp critical
            {
                for (auto entry : entries) {
                    index->insert(entry);
                }
            }
        }
    }

    index->finalize()->save(std::ofstream(opts.index_path, std::ios::binary));

    return 0;
}
