#include "Modules/CalcQueryStats.hpp"

#include <algorithm>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Logging/QueryStatsLogger.hpp"
#include "Util/Stats/QueryStats.hpp"
#include "Util/Types/RunContext.hpp"

void update_query_stats(QueryStats &stats, const MtsQuery &query, const MultivariateTimeSeries &mts) {
    int num_start_pos = 0;
    MtsNumChannelsT num_channels = mts.get_num_channels();
    uint mts_len, query_len = query.get_query_len();

    if (query.is_normalized()) {
        vec<Real> sums(num_channels), sq_sums(num_channels);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (!(query.is_channel_used(c))) {
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
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                if (query.is_channel_used(c)) continue;

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
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    if (query.is_channel_used(c)) continue;

                    sums[c] += mts[c][end_pos] - mts[c][start_pos];
                    sq_sums[c] += mts[c][end_pos] * mts[c][end_pos] - mts[c][start_pos] * mts[c][start_pos];
                }
            }
        }
    } else {
        throw std::runtime_error("Query statistics calculation not implemented for raw queries");
    }
}

void calculate_query_stats(MtsQuerySet &query_set, bool normalized, QueryStatsLogger &logger) {
    auto &dataset = query_set.get_source_dataset();
    auto [num_channels, series_len, num_series, dataset_path] = dataset.get_properties();
    auto num_queries = query_set.get_properties().m_num_queries;
    auto [l_min, l_max] = query_set.get_properties().m_length_range;

    for (uint q = 0; q < num_queries; ++q) {
        auto query = query_set.load_next_query(normalized);
        QueryStats stats;
        for (uint i = 0; i < num_series; ++i) {
            auto mts = dataset.load_next_series(query.get_used_channels());
            update_query_stats(stats, query, mts);
        }

        stats.m_dist_stats.calculate(U(stats.m_subs_count));
        stats.m_rc_using_max = (stats.m_dist_stats.m_max - stats.m_dist_stats.m_min) / stats.m_dist_stats.m_min;
        stats.m_rc_using_mean = stats.m_dist_stats.m_mean / stats.m_dist_stats.m_min;

        logger.write_entry(query_set, query, stats);
    }
}
