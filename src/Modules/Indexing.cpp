#include <filesystem>
#include <fstream>
#include <iostream>

#include "Modules/Indexing.hpp"
#include "Search/IUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxUlisseEnvelopeIndex.hpp"

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

std::unique_ptr<IUlisseEnvelopeIndex> get_index(const IndexOptions &opts) {
    switch (opts.index_params->get_type()) {
        case ISAX_ENVELOPE:
            auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());
            SaxSegIndT num_seg_per_channel = opts.series_len / params->segment_len;

            auto breakpoint_strategy = get_breakpoint_strategy(params);
            auto split_strategy = get_split_strategy(params, num_seg_per_channel, opts.num_channels);

            auto *index = new iSaxUlisseEnvelopeIndex(
                num_seg_per_channel, opts.num_channels, params->first_layer_num_bits, params->leaf_capacity,
                std::move(breakpoint_strategy), std::move(split_strategy), params->num_bits_limit);
            return std::unique_ptr<IUlisseEnvelopeIndex>(index);
    }
    return nullptr;
}

int create_index(const IndexOptions &opts) {
    if (!std::filesystem::exists(opts.dataset_path)) {
        std::cerr << "Error: Dataset " << opts.dataset_path << " does not exist." << std::endl;
        return 1;
    }

    std::ifstream data_file(opts.dataset_path, std::ios::binary);

    unsigned num_series = data_file.tellg() / (opts.num_channels * opts.series_len * sizeof(float));

    auto index = get_index(opts);

    for (size_t i = 0; i < num_series; ++i) {
        vec<vec<float>> mts(opts.num_channels, vec<float>(opts.series_len));
        vec<UlisseEnvelope> envelopes(opts.num_channels);

        for (MtsNumChannelsT c = 0; c < opts.num_channels; ++c) {
            // 1. Read data into channel (enough to create next envelope)
            // 2. Create next envelope
        }
        // 3. Insert envelope into index
    }
    // 4. Finalize index
    // 5. Serialize index

    return 0;
}
