#include "Modules/CsvParsing.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/Stats/ChannelStats.hpp"
#include "Util/Types/RunContext.hpp"

void create_dataset_from_csv(const MtsDataset &dataset, const vec<str> &csv_paths, uint num_series, uint l_min,
                             uint l_max, char col_sep, Real min_subs_sd, const RunContext &run_context) {
    auto [num_channels, series_len, num_series, dataset_file] = dataset.get_properties();
    auto [normalized, seed, data_path, logs_path] = run_context;

    // Check files
    vec<std::ifstream> csv_streams;
    for (str csv_path : csv_paths) {
        csv_streams.emplace_back(csv_path);
        if (!csv_streams.back().is_open()) {
            std::cerr << "Error: Could not open CSV file " << csv_path << '\n';
            std::cerr << "Reason: " << std::strerror(errno) << std::endl;
        }
    }

    // Create dataset directory
    str dataset_path = std::filesystem::path(data_path) / dataset_file;
    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());

    // Build dataset
    std::ofstream dataset_ofs(dataset_path, std::ios::binary);
    if (!dataset_ofs.is_open()) {
        std::cerr << "Error: Could not create dataset " << dataset_path << '\n';
        std::cerr << "Reason: " << std::strerror(errno) << std::endl;
    }

    str line;
    vec<vec<Real>> mts_data(num_channels, vec<Real>(series_len));
    vec<vec<vec<Real>>> all_mts_data;
    MtsNumChannelsT channel_ind = 0;
    bool discard = false;

    while (true) {
        std::getline(csv_streams[channel_ind], line);

        if (!discard) {
            std::istringstream iss(line);
            str value;
            Real sum = 0, sum_sq = 0;
            uint ind = 0;

            while (std::getline(iss, value, col_sep)) {
                // Discard series if a values is not a valid number
                try {
                    mts_data[channel_ind][ind] = R(std::stod(value));
                } catch (const std::exception &e) {
                    discard = true;
                    goto next_channel;
                }

                ++ind;
                // If required, check whether the standard deviation of all relevant-length subsequences is above the
                // provided threshold. If that is not the case, discard the series.
                if (min_subs_sd > 0) {
                    sum += mts_data[channel_ind][ind - 1];
                    sum_sq += mts_data[channel_ind][ind - 1] * mts_data[channel_ind][ind - 1];

                    uint start_min = U(std::max(0, static_cast<int>(ind - l_max)));
                    int start_max = static_cast<int>(ind - l_min);
                    Real sum_tmp = sum, sum_sq_tmp = sum_sq;
                    for (uint start = start_min; static_cast<int>(start) <= start_max; ++start) {
                        Real sigma = calculate_mu_and_sigma(sum_tmp, sum_sq_tmp, U(ind - start)).second;
                        if (sigma < min_subs_sd) {
                            discard = true;
                            goto next_channel;
                        }
                        sum_tmp -= mts_data[channel_ind][start];
                        sum_sq_tmp -= mts_data[channel_ind][start] * mts_data[channel_ind][start];
                    }
                    if (ind >= l_max) {
                        sum -= mts_data[channel_ind][start_min];
                        sum_sq -= mts_data[channel_ind][start_min] * mts_data[channel_ind][start_min];
                    }
                }
                if (ind == series_len) break;
            }
            // Discard series if it does not have enough values
            if (ind < series_len) discard = true;
        }
    next_channel:
        if (++channel_ind == num_channels) {
            if (!discard) all_mts_data.push_back(mts_data);
            channel_ind = 0;
            discard = false;
        }
        if (csv_streams[channel_ind].eof()) break;
    }

    if (all_mts_data.empty()) {
        throw std::runtime_error("Error: No valid time series found in the dataset");
    }

    // Select a subset of all non-discarded time series
    std::default_random_engine generator(seed);

    vec<uint> mts_indexes(all_mts_data.size());
    std::iota(mts_indexes.begin(), mts_indexes.end(), 0);
    std::shuffle(mts_indexes.begin(), mts_indexes.end(), generator);
    if (mts_indexes.size() > num_series) mts_indexes.resize(num_series);

    vec<Real> sums(num_channels, R(0.0)), sum_sqs(num_channels, R(0.0));
    for (uint mts_ind : mts_indexes) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto &ts = all_mts_data[mts_ind][c];
            dataset_ofs.write(reinterpret_cast<const char *>(ts.data()), sizeof(Real) * series_len);
            for (uint i = 0; i < series_len; ++i) {
                sums[c] += ts[i];
                sum_sqs[c] += ts[i] * ts[i];
            }
        }
    }

    // Save channel statistics
    ChannelStats(sums, sum_sqs, series_len, U(mts_indexes.size())).save(dataset.get_channel_stats_path());

    // Log dataset creation
    DatasetLogger::write_entry(
        std::make_unique<CsvDatasetLogAttributes>(csv_paths, mts_indexes.size(), l_min, l_max, min_subs_sd, seed),
        dataset, logs_path);
}
