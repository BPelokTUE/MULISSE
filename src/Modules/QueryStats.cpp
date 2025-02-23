#include "Modules/QueryStats.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

int calculate_query_stats(bool normalized) {
    auto &RS = RunSettings::get_instance();
    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    std::ifstream query_ifs(RS.get_query_path());

    MtsNumChannelsT num_channels = RS.get_dataset_props().num_channels;
    vec<vec<float>> query(num_channels);

    size_t query_count = 0;

    for (MtsNumChannelsT c = 0; !query_ifs.eof(); c = (c + 1) % num_channels) {
        str line;
        std::getline(query_ifs, line);
        std::istringstream iss(line);
        float value, sum = 0, sq_sum = 0;

        query[c].clear();
        while (iss >> value) {
            query[c].push_back(value);
            sum += value;
            sq_sum += value * value;
        }
        if (normalized && query[c].size() > 0) {
            auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, query[c].size());
            for (size_t i = 0; i < query[c].size(); ++i) query[c][i] = (query[c][i] - mu) / sigma;
        }

        if (c == num_channels - 1) {
            dataset_ifs.seekg(0);
            QueryStats stats = {
                .min_dist = INF,
                .max_dist = 0,
                .mean_dist = 0,
                .dist_std_dev = 0,
            };

            // TODO

            QueryStatsLogger::write_entry(query_count, query, stats);
        }
    }
    return 0;
}
