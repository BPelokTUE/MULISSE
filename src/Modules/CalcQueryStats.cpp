#include "Modules/CalcQueryStats.hpp"

#include <algorithm>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Logging/QueryStatsLogger.hpp"
#include "Util/Types/RunContext.hpp"

void update_query_stats(QueryStats &stats, const vec<vec<Real>> &query, const vec<vec<Real>> &mts, bool normalized) {
    int num_start_pos = 0;
    uint mts_len, query_len;

    if (normalized) {
        vec<Real> sums(query.size()), sq_sums(query.size());
        for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
            if (!(query[c].empty())) {
                query_len = U(query[c].size());
                mts_len = U(mts[c].size());
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

                auto [mu, sigma] = calculate_mu_and_sigma(sums[c], sq_sums[c], U(query_len));
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

int calculate_query_stats(const MtsDataset &dataset, const MtsQuerySet &query_set, const RunContext &run_context) {
    auto [num_channels, series_len, num_series, dataset_path] = dataset.get_properties();
    auto [num_queries, length_range, query_set_path] = query_set.get_properties();
    auto [normalized, seed, data_path, logs_path] = run_context;

    std::ifstream dataset_ifs(std::filesystem::path(data_path) / dataset_path, std::ios::binary);
    std::ifstream query_ifs(std::filesystem::path(data_path) / query_set_path);

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
            auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, U(query[c].size()));
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

            stats.m_dist_stats.calculate(U(stats.m_subs_count));
            stats.m_rc_using_max = (stats.m_dist_stats.m_max - stats.m_dist_stats.m_min) / stats.m_dist_stats.m_min;
            stats.m_rc_using_mean = stats.m_dist_stats.m_mean / stats.m_dist_stats.m_min;

            QueryStatsLogger::write_entry(query_count++, query, stats, normalized);
        }
    }
    return 0;
}
