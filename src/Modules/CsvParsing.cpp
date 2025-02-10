#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

#include "Modules/CsvParsing.hpp"
#include "Util/RunSettings.hpp"
#include "Util/typedefs.hpp"

int create_dataset_from_csv(const vec<str> &csv_paths, char col_sep) {
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
    uint length = series_len;

    while (true) {
        auto &csv_ifs = csv_streams[channel];
        std::getline(csv_ifs, line);

        if (length == series_len) {
            std::istringstream iss(line);
            str value;
            uint ind = 0;
            while (std::getline(iss, value, col_sep)) {
                mts[channel][ind++] = std::stof(value);
                if (ind == series_len) break;
            }
            length = std::min(length, ind);
        }

        if (++channel == num_channels) {
            if (length == series_len) {
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    dataset_ofs.write(reinterpret_cast<const char *>(mts[c].data()), sizeof(float) * series_len);
                }
            }
            channel = 0;
            length = series_len;
        }

        if (csv_ifs.eof()) break;
    }
    return 0;
}
