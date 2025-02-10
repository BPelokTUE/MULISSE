#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

#include "Modules/CsvParsing.hpp"
#include "Util/RunSettings.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

int create_dataset_from_csv(const vec<str> &csv_paths, uint low_sd_len, char col_sep) {
    for (str csv_path : csv_paths) {
        if (!std::filesystem::exists(csv_path)) {
            std::cerr << "Error: Dataset " << csv_path << " does not exist\n";
            return 1;
        }
    }

    vec<std::ifstream> csv_streams;
    for (str csv_path : csv_paths) {
        csv_streams.emplace_back(csv_path);
        if (!csv_streams.back()) {
            std::cerr << "Error: Could not open CSV file " << csv_path << '\n';
            return 2;
        }
    }

    auto &RS = RunSettings::get_instance();
    MtsNumChannelsT num_channels = csv_paths.size();
    uint series_len = RS.get_dataset_props().series_len;
    str dataset_path = RS.get_dataset_path();

    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());

    std::ofstream dataset_ofs(dataset_path, std::ios::binary);
    if (!dataset_ofs) {
        std::cerr << "Error: Could not create dataset " << RS.get_dataset_path() << '\n';
        std::cerr << "Reason: " << std::strerror(errno) << std::endl;
        return 3;
    }

    str line;
    vec<vec<float>> mts(num_channels, vec<float>(series_len));
    MtsNumChannelsT channel = 0;
    bool discard = false;
    uint length = series_len;

    while (true) {
        auto &csv_ifs = csv_streams[channel];
        std::getline(csv_ifs, line);

        if (!discard) {
            std::istringstream iss(line);
            str value;
            uint ind = 0;
            float sum = 0, sum_sq = 0;

            while (std::getline(iss, value, col_sep)) {
                try {
                    mts[channel][ind] = std::stof(value);
                } catch (const std::exception &e) {
                    discard = true;
                    break;
                }
                sum += mts[channel][ind];
                sum_sq += mts[channel][ind] * mts[channel][ind];

                ++ind;
                if (ind >= low_sd_len) {
                    float sigma = calculate_mu_and_sigma(sum, sum_sq, ind).second;
                    if (sigma < MIN_SUBS_SIGMA) {
                        discard = true;
                        break;
                    }
                    sum -= mts[channel][ind - low_sd_len];
                    sum_sq -= mts[channel][ind - low_sd_len] * mts[channel][ind - low_sd_len];
                }
                if (ind == series_len) break;
            }
            if (ind < series_len) discard = true;
        }

        if (++channel == num_channels) {
            if (!discard) {
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    dataset_ofs.write(reinterpret_cast<const char *>(mts[c].data()), sizeof(float) * series_len);
                }
            }
            channel = 0;
            length = series_len;
            discard = false;
        }

        if (csv_ifs.eof()) break;
    }
    return 0;
}
