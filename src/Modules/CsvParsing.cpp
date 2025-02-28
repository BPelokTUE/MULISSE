#include <algorithm>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <random>

#include "Modules/CsvParsing.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"

int create_dataset_from_csv(const vec<str> &csv_paths, uint num_series, uint low_sd_len, int seed, char col_sep) {
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
    vec<vec<vec<float>>> all_mts;
    MtsNumChannelsT channel = 0;
    bool discard = false;
    uint length = series_len, ts_ind = 0;

    while (true) {
        std::getline(csv_streams[channel], line);

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
            if (!discard) all_mts.push_back(mts);
            channel = 0;
            length = series_len;
            discard = false;
            ++ts_ind;
        }
        if (csv_streams[channel].eof() || (ts_ind > 2 * num_series && all_mts.size() >= num_series)) break;
    }

    if (all_mts.empty()) {
        throw std::runtime_error("Error: No valid time series found in the dataset");
    }

    std::default_random_engine generator(seed);

    vec<uint> mts_indexes(all_mts.size());
    std::iota(mts_indexes.begin(), mts_indexes.end(), 0);
    std::shuffle(mts_indexes.begin(), mts_indexes.end(), generator);
    if (mts_indexes.size() > num_series) mts_indexes.resize(num_series);

    for (uint mts_ind : mts_indexes) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            dataset_ofs.write(reinterpret_cast<const char *>(all_mts[mts_ind][c].data()), sizeof(float) * series_len);
        }
    }

    DatasetLogger::write_entry(
        std::make_unique<CsvDatasetLogAttributes>(csv_paths, mts_indexes.size(), low_sd_len, seed));

    return 0;
}
