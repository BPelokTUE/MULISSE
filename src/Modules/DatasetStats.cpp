#include "Modules/DatasetStats.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Logging/DatasetStatsLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

using namespace boost::accumulators;

int calculate_dataset_stats(uint num_lags) {
    auto &RS = RunSettings::get_instance();
    auto [num_channels, series_len, num_series, file] = RS.get_dataset_props();

    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    for (uint i = 0; i < num_series; ++i) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            accumulator_set<Real, stats<tag::mean, tag::variance, tag::skewness, tag::kurtosis>> channel_stats;

            vec<Real> channel(series_len);
            dataset_ifs.read(reinterpret_cast<char *>(channel.data()), series_len * sizeof(Real));
            for (auto val : channel) channel_stats(val);

            vec<uint> tv_lags, ac_lags;
            tv_lags.reserve(num_lags);
            ac_lags.reserve(num_lags);
            for (uint li = 0; li < num_lags; ++li) {
                tv_lags.push_back(li + 1);
                ac_lags.push_back(series_len * (li + 1) / (num_lags + 1));
            }

            Real mu = mean(channel_stats), sigma = sqrt(variance(channel_stats));
            vec<accumulator_set<Real, stats<tag::mean, tag::variance>>> total_var_stats(num_lags);
            vec<accumulator_set<Real, stats<tag::mean, tag::variance>>> autocorr_stats(num_lags);
            for (uint j = 0; j < series_len; ++j) {
                channel[j] = (channel[j] - mu) / sigma;
                for (uint li = 0; li < num_lags; ++li) {
                    if (tv_lags[li] <= j) total_var_stats[li](std::abs(channel[j] - channel[j - tv_lags[li]]));
                    if (ac_lags[li] <= j) autocorr_stats[li](channel[j] * channel[j - ac_lags[li]]);
                }
            }
            vec<Real> total_var_means(num_lags), total_var_stds(num_lags), autocorr_means(num_lags),
                autocorr_stds(num_lags);
            for (uint li = 0; li < num_lags; ++li) {
                total_var_means[li] = mean(total_var_stats[li]);
                total_var_stds[li] = sqrt(variance(total_var_stats[li]));
                autocorr_means[li] = mean(autocorr_stats[li]);
                autocorr_stds[li] = sqrt(variance(autocorr_stats[li]));
            }

            DatasetStats stats{mu,
                               sigma,
                               skewness(channel_stats),
                               kurtosis(channel_stats),
                               total_var_means,
                               total_var_stds,
                               autocorr_means,
                               autocorr_stds};
            DatasetStatsLogger::write_entry(file, i, c, stats);
        }
    }
    return 0;
}
