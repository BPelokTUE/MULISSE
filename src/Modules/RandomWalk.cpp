#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>

#include "Modules/RandomWalk.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

int create_random_walks(Real step_sigma, bool zero_start, uint seed) {
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();
    auto [file, num_channels, series_len, num_series] = RS.get_dataset_props();
    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());

    if (std::filesystem::exists(dataset_path)) {
        std::cerr << "Error: Dataset " << dataset_path << " already exists\n";
        return 1;
    }

    // Create binary file containing the time series
    std::ofstream outfile(dataset_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not create dataset " << dataset_path << '\n';
        std::cerr << "Reason: " << std::strerror(errno) << std::endl;
        return 2;
    }

    std::default_random_engine generator(seed);
    std::normal_distribution<Real> distribution(0.0, step_sigma);

    for (uint i = 0; i < num_series; ++i) {
        for (uint j = 0; j < num_channels; ++j) {
            Real value = zero_start ? 0 : distribution(generator);
            for (uint k = 0; k < series_len; ++k) {
                value += distribution(generator);
                // Cast the reference to `value` into `const char` pointer, so `outfile.write` will
                // try to write the bytes stored in `value` as chars. By definition a `char` contains
                // a single byte, so `sizeof(value)` can be used to specify how many chars to write.
                outfile.write(reinterpret_cast<const char *>(&value), sizeof(value));
            }
        }
    }

    outfile.close();

    DatasetLogger::write_entry(std::make_unique<RandomWalkLogAttributes>(step_sigma, seed));

    return 0;
}
