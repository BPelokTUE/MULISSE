#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "Modules/QueryGen.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"

int create_queries(float noise, uint series_len, uint num_channels, uint num_queries, vec<uint> lengths, int seed) {
    auto &RS = RunSettings::get_instance();
    const str &dataset_path = RS.get_dataset_path();
    const str &query_path = RS.get_query_path();

    if (!std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " does not exist." << std::endl;
        return 1;
    }

    // Extract time series from dataset
    std::default_random_engine rng(seed);
    std::normal_distribution<float> noise_normal_dist(0.0, noise);

    uint num_series = get_dataset_size(dataset_path) / (num_channels * series_len * sizeof(float));
    std::uniform_int_distribution<uint> series_uniform_dist(0, num_series - 1), channel_uniform_dist(1, num_channels);
    vec<std::uniform_int_distribution<uint>> start_pos_dists(lengths.size());
    for (uint i = 0; i < lengths.size(); ++i) {
        start_pos_dists[i] = std::uniform_int_distribution<uint>(0, series_len - lengths[i]);
    }

    std::ifstream data_file(dataset_path, std::ios::binary);
    std::ofstream query_file(query_path);

    // vector of tuples of start position to extract from, length of the query, and channels to include
    vec<std::tuple<FilePositionT, uint, vec<bool>>> query_descriptors(lengths.size() * num_queries);
    for (uint i = 0; i < num_queries; ++i) {
        for (uint j = 0; j < lengths.size(); ++j) {
            vec<bool> channels(num_channels, false);
            uint included_channels = channel_uniform_dist(rng);
            std::fill(channels.begin(), channels.begin() + included_channels, true);
            std::shuffle(channels.begin(), channels.end(), rng);

            FilePositionT series_ind = series_uniform_dist(rng), start_pos = start_pos_dists[j](rng);
            FilePositionT file_pos = series_ind * series_len * num_channels + start_pos;
            query_descriptors[i * lengths.size() + j] = {file_pos * sizeof(float), lengths[j], channels};
        }
    }
    std::sort(query_descriptors.begin(), query_descriptors.end());

    for (size_t q = 0; q < query_descriptors.size(); ++q) {
        const auto &[start_offset, length, channels] = query_descriptors[q];
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (channels[c]) {
                data_file.seekg(start_offset + c * series_len * sizeof(float));
                float value;
                for (uint j = 0; j < length; ++j) {
                    data_file.read(reinterpret_cast<char *>(&value), sizeof(value));
                    value += noise_normal_dist(rng);
                    query_file << value;
                    if (j < length - 1) query_file << ' ';
                }
            }
            if (q < query_descriptors.size() - 1 || c < num_channels - 1) {
                query_file << '\n';
            }
        }
    }

    return 0;
}
