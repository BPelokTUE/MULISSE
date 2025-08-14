#include "Modules/RandomWalk.hpp"

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/Stats/ChannelStats.hpp"

void create_random_walks(const MtsDataset &dataset, Real step_sigma, bool zero_start, uint seed) {
    auto [num_channels, series_len, num_series, dataset_path] = dataset.get_settings();
    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());

    // Create binary file containing the time series
    std::ofstream outfile(dataset_path, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not create dataset file: " + dataset_path + " - " + std::strerror(errno));
    }

    std::default_random_engine generator(seed);
    std::normal_distribution<Real> distribution(0.0, step_sigma);

    vec<Real> sums(num_channels, R(0.0)), sum_sqs(num_channels, R(0.0));

    for (uint i = 0; i < num_series; ++i) {
        for (uint j = 0; j < num_channels; ++j) {
            Real value = zero_start ? 0 : distribution(generator);
            for (uint k = 0; k < series_len; ++k) {
                value += distribution(generator);
                sums[j] += value;
                sum_sqs[j] += value * value;
                // Cast the reference to `value` into `const char` pointer, so `outfile.write` will
                // try to write the bytes stored in `value` as chars. By definition a `char` contains
                // a single byte, so `sizeof(value)` can be used to specify how many chars to write.
                outfile.write(reinterpret_cast<const char *>(&value), sizeof(value));
            }
        }
    }
    ChannelStats(sums, sum_sqs, series_len, num_series).save(dataset.get_channel_stats_path());

    DatasetLogger::write_entry(std::make_unique<RandomWalkLogAttributes>(step_sigma, seed));
}
