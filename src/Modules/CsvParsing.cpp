#include "Modules/CsvParsing.hpp"

#include <random>

#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

int create_dataset_from_csv(const vec<str> &csv_paths, uint num_series, uint l_min, uint l_max, uint seed,
                            char col_sep) {
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
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(csv_paths.size());
    uint series_len = RS.get_dataset_props().m_series_len;
    str dataset_path = RS.get_dataset_path();

    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());

    std::ofstream dataset_ofs(dataset_path, std::ios::binary);
    if (!dataset_ofs) {
        std::cerr << "Error: Could not create dataset " << RS.get_dataset_path() << '\n';
        std::cerr << "Reason: " << std::strerror(errno) << std::endl;
        return 3;
    }

    str line;
    vec<vec<Real>> mts(num_channels, vec<Real>(series_len));
    vec<vec<vec<Real>>> all_mts;
    MtsNumChannelsT channel = 0;
    bool discard = false;

    while (true) {
        std::getline(csv_streams[channel], line);

        if (!discard) {
            std::istringstream iss(line);
            str value;
            Real sum = 0, sum_sq = 0;
            uint ind = 0;

            while (std::getline(iss, value, col_sep)) {
                try {
                    mts[channel][ind] = std::stof(value);
                } catch (const std::exception &e) {
                    discard = true;
                    goto next_channel;
                }
                sum += mts[channel][ind];
                sum_sq += mts[channel][ind] * mts[channel][ind];

                ++ind;
                uint start_min = U(std::max(0, static_cast<int>(ind - l_max)));
                int start_max = static_cast<int>(ind - l_min);
                Real sum_tmp = sum, sum_sq_tmp = sum_sq;
                for (uint start = start_min; static_cast<int>(start) <= start_max; ++start) {
                    Real sigma = calculate_mu_and_sigma(sum_tmp, sum_sq_tmp, U(ind - start)).second;
                    if (sigma < MIN_SUBS_SIGMA) {
                        discard = true;
                        goto next_channel;
                    }
                    sum_tmp -= mts[channel][start];
                    sum_sq_tmp -= mts[channel][start] * mts[channel][start];
                }
                if (ind >= l_max) {
                    sum -= mts[channel][start_min];
                    sum_sq -= mts[channel][start_min] * mts[channel][start_min];
                }
                if (ind == series_len) break;
            }
            if (ind < series_len) discard = true;
        }
    next_channel:
        if (++channel == num_channels) {
            if (!discard) all_mts.push_back(mts);
            channel = 0;
            discard = false;
        }
        if (csv_streams[channel].eof()) break;
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
            dataset_ofs.write(reinterpret_cast<const char *>(all_mts[mts_ind][c].data()), sizeof(Real) * series_len);
        }
    }

    DatasetLogger::write_entry(
        std::make_unique<CsvDatasetLogAttributes>(csv_paths, mts_indexes.size(), l_min, l_max, seed));

    return 0;
}
