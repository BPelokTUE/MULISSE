#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "Modules/RandomWalk.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

int create_random_walks(float rw_noise, bool zero_start, uint num_series, uint series_len, uint num_channels,
                        int seed) {
    str dataset_path = RunSettings::get_instance().get_dataset_path();

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
    std::normal_distribution<float> distribution(0.0, rw_noise);

    for (uint i = 0; i < num_series; ++i) {
        for (uint j = 0; j < num_channels; ++j) {
            float value = zero_start ? 0 : distribution(generator);
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

    DatasetLogger::write_entry();

    return 0;
}
