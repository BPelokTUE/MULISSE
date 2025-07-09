#include "Modules/RandomWalk.hpp"

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

int create_random_walks(Real step_sigma, bool zero_start, uint seed) {
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();
    auto [num_channels, series_len, num_series, file] = RS.get_dataset_props();
    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());

    // Create binary file containing the time series
    std::ofstream outfile(dataset_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not create dataset " << dataset_path << '\n';
        std::cerr << "Reason: " << std::strerror(errno) << std::endl;
        return 1;
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
    RS.calc_and_save_channel_stats(sums, sum_sqs, series_len, num_series);

    DatasetLogger::write_entry(std::make_unique<RandomWalkLogAttributes>(step_sigma, seed));
    return 0;
}
