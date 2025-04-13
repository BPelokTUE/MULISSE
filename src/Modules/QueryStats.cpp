#include <algorithm>
#include <fstream>

#include "Modules/QueryStats.hpp"
#include "Util/constants.hpp"
#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logging/QueryStatsLogger.hpp"
#include "Util/RunSettings.hpp"

void update_query_stats(QueryStats &stats, const vec<vec<Real>> &query, const vec<vec<Real>> &mts, bool normalized) {
    int num_start_pos = 0;
    uint mts_len, query_len;

    if (normalized) {
        vec<Real> sums(query.size()), sq_sums(query.size());
        for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
            if (!(query[c].empty())) {
                query_len = static_cast<uint>(query[c].size());
                mts_len = static_cast<uint>(mts[c].size());
                num_start_pos = static_cast<int>(mts[c].size() - query_len + 1);

                for (size_t i = 0; i < query_len; ++i) {
                    sums[c] += mts[c][i];
                    sq_sums[c] += mts[c][i] * mts[c][i];
                }
            }
        }

        for (uint start_pos = 0; static_cast<int>(start_pos) < num_start_pos; ++start_pos) {
            Real dist_squared = 0;
            for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
                if (query[c].empty()) continue;

                auto [mu, sigma] = calculate_mu_and_sigma(sums[c], sq_sums[c], static_cast<uint>(query_len));
                for (uint i = 0; i < query_len; ++i) {
                    Real diff = (mts[c][start_pos + i] - mu) / sigma - query[c][i];
                    dist_squared += diff * diff;
                }
            }
            Real dist = std::sqrt(dist_squared);
            stats.m_dist_stats.update(dist);
            stats.m_subs_count++;

            uint end_pos = start_pos + query_len;
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
    vec<vec<Real>> query(num_channels);

    uint query_count = 0;
    for (MtsNumChannelsT c = 0; !query_ifs.eof(); c = static_cast<MtsNumChannelsT>((c + 1) % num_channels)) {
        str line;
        std::getline(query_ifs, line);
        std::istringstream iss(line);
        Real value, sum = 0, sq_sum = 0;

        query[c].clear();
        while (iss >> value) {
            query[c].push_back(value);
            sum += value;
            sq_sum += value * value;
        }
        if (normalized && query[c].size() > 0) {
            auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, static_cast<uint>(query[c].size()));
            for (size_t i = 0; i < query[c].size(); ++i) query[c][i] = (query[c][i] - mu) / sigma;
        }

        if (c == num_channels - 1) {
            dataset_ifs.seekg(0);
            QueryStats stats;
            for (uint i = 0; i < num_series; ++i) {
                vec<vec<Real>> mts(num_channels);
                for (MtsNumChannelsT cc = 0; cc < num_channels; ++cc) {
                    if (!query[cc].empty()) {
                        mts[cc].resize(series_len);
                        dataset_ifs.read(reinterpret_cast<char *>(mts[cc].data()), series_len * sizeof(Real));
                    } else {
                        dataset_ifs.seekg(series_len * sizeof(Real), std::ios::cur);
                    }
                }
                update_query_stats(stats, query, mts, normalized);
            }

            stats.m_dist_stats.calculate(static_cast<uint>(stats.m_subs_count));
            stats.m_rc_using_max = (stats.m_dist_stats.m_max - stats.m_dist_stats.m_min) / stats.m_dist_stats.m_min;
            stats.m_rc_using_mean = stats.m_dist_stats.m_mean / stats.m_dist_stats.m_min;

            QueryStatsLogger::write_entry(query_count++, query, stats, normalized);
        }
    }
    return 0;
}
