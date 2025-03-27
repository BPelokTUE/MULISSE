#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "Modules/QueryGen.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

struct QueryDescriptor {
    SubsequenceInfo subs_info;
    uint length;
    vec<bool> channels;

    bool operator<(const QueryDescriptor &other) const { return subs_info < other.subs_info; }
};

int create_queries(QuerySetOptions opts) {
    auto &RS = RunSettings::get_instance();
    const str &dataset_path = RS.get_dataset_path();
    const str &query_path = RS.get_query_path();
    uint num_channels = RS.get_dataset_props().num_channels;
    uint series_len = RS.get_dataset_props().series_len;

    if (!std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " does not exist." << std::endl;
        return 1;
    }

    // Check channel selection
    if (!opts.channel_mask.empty()) {
        if (opts.channel_mask.size() != num_channels) {
            std::cerr << "Error: Channel mask must have the same number of elements as the number of channels in the "
                         "dataset ("
                      << num_channels << "), but has " << opts.channel_mask.size() << '\n';
            return 2;
        }
    } else if (opts.used_channels != 0 && opts.used_channels > num_channels) {
        std::cerr << "Error: Number of used channels (" << opts.used_channels
                  << ") must be less than or equal to the number of channels in the dataset (" << num_channels << ")\n";
        return 3;
    }

    // Check length specification
    if ((opts.l_min == 0 || opts.l_max < opts.l_min) && opts.exact_lengths.empty()) {
        std::cerr << "Error: Either a list of exact lengths or a minimum and maximum length must be provided\n";
        return 4;
    }

    // Extract time series from dataset
    std::default_random_engine rng(opts.seed);
    std::normal_distribution<Real> noise_normal_dist(0.0, opts.noise);

    uint num_series = get_dataset_size(dataset_path) / (num_channels * series_len * sizeof(Real));
    std::uniform_int_distribution<uint> series_uniform_dist(0, num_series - 1), channel_uniform_dist(1, num_channels),
        length_uniform_dist(opts.l_min, opts.l_max);

    std::ifstream data_file(dataset_path, std::ios::binary);
    std::ofstream query_file(query_path);

    bool random_lengths = (opts.l_min > 0 && opts.l_max >= opts.l_min);
    uint total_num_queries = random_lengths ? opts.num_queries : opts.num_queries * opts.exact_lengths.size();
    vec<QueryDescriptor> query_descriptors(total_num_queries);

    auto generate_query_descriptor = [&](uint length) -> QueryDescriptor {
        uint included_channels = opts.used_channels == 0 ? channel_uniform_dist(rng) : opts.used_channels;
        vec<bool> channels(num_channels, false);
        if (opts.channel_mask.empty()) {
            std::fill(channels.begin(), channels.begin() + included_channels, true);
            std::shuffle(channels.begin(), channels.end(), rng);
        } else {
            channels = opts.channel_mask;
        }

        auto start_pos_dist = std::uniform_int_distribution<uint>(0, series_len - length);
        SubsequenceInfo subs_info = {series_uniform_dist(rng), start_pos_dist(rng)};
        return {subs_info, length, channels};
    };

    for (uint i = 0; i < opts.num_queries; ++i) {
        if (random_lengths) {
            query_descriptors[i] = generate_query_descriptor(length_uniform_dist(rng));
        } else {
            for (uint j = 0; j < opts.exact_lengths.size(); ++j) {
                query_descriptors[i * opts.exact_lengths.size() + j] = generate_query_descriptor(opts.exact_lengths[j]);
            }
        }
    }
    std::sort(query_descriptors.begin(), query_descriptors.end());

    for (size_t q = 0; q < query_descriptors.size(); ++q) {
        const auto &[subs_info, length, channels] = query_descriptors[q];
        SubsequenceInfo series_start = {subs_info.series_ind, 0, series_len};

        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (channels[c]) {
                data_file.seekg(series_start.get_file_pos(series_len, num_channels, c));
                Real sum = 0, sum_sq = 0, value;
                for (uint j = 0; j < series_len; ++j) {
                    data_file.read(reinterpret_cast<char *>(&value), sizeof(value));
                    sum += value;
                    sum_sq += value * value;
                }
                Real sigma = calculate_mu_and_sigma(sum, sum_sq, series_len).second;

                data_file.seekg(subs_info.get_file_pos(series_len, num_channels, c));
                for (uint j = 0; j < length; ++j) {
                    data_file.read(reinterpret_cast<char *>(&value), sizeof(value));
                    value += noise_normal_dist(rng) * sigma;
                    query_file << value;
                    if (j < length - 1) query_file << ' ';
                }
            }
            if (q < query_descriptors.size() - 1 || c < num_channels - 1) {
                query_file << '\n';
            }
        }
    }

    auto opts_to_log = opts;
    opts_to_log.num_queries = total_num_queries;
    if (random_lengths) {
        opts_to_log.exact_lengths = vec<uint>{};
    } else {
        opts_to_log.l_min = 0;
        opts_to_log.l_max = 0;
    }
    opts_to_log.used_channels = opts.channel_mask.empty() ? opts.used_channels : 0;
    QuerySetLogger::write_entry(opts_to_log);

    return 0;
}
