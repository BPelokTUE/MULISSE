#include <fstream>

#include "Search/SequentialScan.hpp"
#include "Util/RunSettings.hpp"

vec<SearchResult> SequentialScan::search(const vec<vec<float>> &query, const SearchOptions &opts) const {
    auto [dataset_path, num_channels, series_len, num_series] = RunSettings::get_instance().get_dataset_props();

    std::ifstream data_stream(dataset_path, std::ios::binary);

    for (uint i = 0; i < num_series; ++i) {
        vec<vec<float>> mts(num_channels);
        for (uint c = 0; c < num_channels; ++c) {
            mts[c].resize(series_len);
            data_stream.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(float));
        }
        FilePositionT file_pos = i * series_len * num_channels;
        opts.distance_measure->update_result_set(opts.result_set.get(), file_pos, query, mts);
    }
    return opts.result_set.get()->get_results();
}
