#include <algorithm>
#include <fstream>

#include "Modules/QueryStats.hpp"
#include "Util/constants.hpp"
#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

void update_query_stats(QueryStats &stats, const vec<vec<float>> &query, const vec<vec<float>> &mts, bool normalized) {
    int num_start_pos = 0, mts_len, query_len;

    if (normalized) {
        vec<DistanceT> sums(query.size()), sq_sums(query.size());
        for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
            if (!(query[c].empty())) {
                query_len = query[c].size();
                mts_len = mts[c].size();
                num_start_pos = mts[c].size() - query_len + 1;

                for (size_t i = 0; i < query_len; ++i) {
                    sums[c] += mts[c][i];
                    sq_sums[c] += mts[c][i] * mts[c][i];
                }
            }
        }

        for (int start_pos = 0; start_pos < num_start_pos; ++start_pos) {
            DistanceT dist_squared = 0;
            for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
                if (query[c].empty()) continue;

                auto [mu, sigma] = calculate_mu_and_sigma(sums[c], sq_sums[c], query_len);
                for (uint i = 0; i < query_len; ++i) {
                    DistanceT diff = (mts[c][start_pos + i] - mu) / sigma - query[c][i];
                    dist_squared += diff * diff;
                }
            }
            float dist = std::sqrt(dist_squared);
            stats.min_dist = std::min(stats.min_dist, dist);
            stats.max_dist = std::max(stats.max_dist, dist);
            stats.mean_dist += dist;
            stats.mean_sq_dist += dist_squared;
            stats.subs_count++;

            int end_pos = start_pos + query_len;
            if (end_pos < mts_len) {
                for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
                    if (query[c].empty()) continue;

                    sums[c] += mts[c][end_pos] - mts[c][start_pos];
                    sq_sums[c] += mts[c][end_pos] * mts[c][end_pos] - mts[c][start_pos] * mts[c][start_pos];
                }
            }
        }
    } else {
        throw std::runtime_error("Non-normalized Euclidean distance not implemented yet");
    }
}

int calculate_query_stats(bool normalized) {
    auto &RS = RunSettings::get_instance();
    auto [file, num_channels, series_len, num_series] = RS.get_dataset_props();

    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    std::ifstream query_ifs(RS.get_query_path());
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
            };
            for (uint i = 0; i < num_series; ++i) {
                vec<vec<float>> mts(num_channels);
                for (uint c = 0; c < num_channels; ++c) {
                    if (!query[c].empty()) {
                        mts[c].resize(series_len);
                        dataset_ifs.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(float));
                    } else {
                        dataset_ifs.seekg(series_len * sizeof(float), std::ios::cur);
                    }
                }
                update_query_stats(stats, query, mts, normalized);
            }
            auto [dist_mean, std_dist] = calculate_mu_and_sigma(stats.mean_dist, stats.mean_sq_dist, stats.subs_count);
            stats.mean_dist = dist_mean;
            stats.std_dist = std_dist;
            stats.rc_using_max = (stats.max_dist - stats.min_dist) / stats.min_dist;
            stats.rc_using_mean = stats.mean_dist / stats.min_dist;

            QueryStatsLogger::write_entry(query_count++, query, stats, normalized);
        }
    }
    return 0;
}
