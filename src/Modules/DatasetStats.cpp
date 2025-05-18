#include "Modules/DatasetStats.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "Util/Logging/DatasetStatsLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

using namespace boost::accumulators;

int calculate_dataset_stats() {
    auto &RS = RunSettings::get_instance();
    auto [num_channels, series_len, num_series, file] = RS.get_dataset_props();

    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    for (uint i = 0; i < num_series; ++i) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            accumulator_set<Real, stats<tag::mean, tag::variance, tag::skewness, tag::kurtosis>> channel_stats;

            vec<Real> channel(series_len);
            dataset_ifs.read(reinterpret_cast<char *>(channel.data()), series_len * sizeof(Real));
            for (auto val : channel) channel_stats(val);

            DatasetStats stats{
                mean(channel_stats),
                sqrt(variance(channel_stats)),
                skewness(channel_stats),
                kurtosis(channel_stats),
            };
            DatasetStatsLogger::write_entry(file, i, c, stats);
        }
    }

    return 0;
}
