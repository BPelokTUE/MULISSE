#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "Util/QueryGen.hpp"

int create_queries(std::string dataset_path, std::string query_path, float noise, unsigned series_len,
                   unsigned num_channels, unsigned num_queries, std::vector<unsigned> lengths) {
    if (!std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " does not exist." << std::endl;
        return 1;
    }

    if (std::filesystem::exists(query_path)) {
        std::cerr << "Error: Query file " << query_path << " already exists." << std::endl;
        return 2;
    }

    // Extract time series from dataset
    std::ifstream data_file(dataset_path, std::ios::binary);
    std::ofstream query_file(query_path);

    std::default_random_engine rng;
    std::normal_distribution<float> noise_normal_dist(0.0, noise);

    unsigned num_series = data_file.tellg() / (num_channels * series_len * sizeof(float));
    std::uniform_int_distribution<unsigned> series_uniform_dist(0, num_series - 1),
        channel_uniform_dist(1, num_channels);

    // vector of tuples of series index to extract from, length of the query, and channels to include
    std::vector<std::tuple<unsigned, unsigned, std::vector<bool>>> query_descriptors(num_queries);
    for (unsigned i = 0; i < num_queries; ++i) {
        for (unsigned length : lengths) {
            std::vector<bool> channels(num_channels, false);
            unsigned included_channels = channel_uniform_dist(rng);
            std::fill(channels.begin(), channels.begin() + included_channels, true);
            std::shuffle(channels.begin(), channels.end(), rng);

            query_descriptors[i] = {series_uniform_dist(rng), length, channels};
        }
    }
    std::sort(query_descriptors.begin(), query_descriptors.end());

    for (size_t q = 0; q < query_descriptors.size(); ++q) {
        const auto &[series_idx, length, channels] = query_descriptors[q];
        unsigned long long start_offset = series_idx * num_channels * series_len * sizeof(float);
        for (unsigned i = 0; i < num_channels; ++i) {
            if (channels[i]) {
                data_file.seekg(start_offset + i * series_len * sizeof(float));
                float value;
                for (unsigned j = 0; j < length; ++j) {
                    data_file.read(reinterpret_cast<char *>(&value), sizeof(value));
                    value += noise_normal_dist(rng);
                    query_file << value;
                    if (j < length - 1) query_file << ' ';
                }
            }
            if (q < query_descriptors.size() - 1 || i < num_channels - 1) {
                query_file << '\n';
            }
        }
    }

    return 0;
}
