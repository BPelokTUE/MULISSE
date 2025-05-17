#include "Modules/DatasetStats.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "Util/RunSettings/RunSettings.hpp"

int calculate_dataset_stats() {
    auto &RS = RunSettings::get_instance();
    auto [num_channels, series_len, num_series, file] = RS.get_dataset_props();

    std::vector<boost::accumulators::accumulator_set<
        Real, boost::accumulators::stats<boost::accumulators::tag::mean, boost::accumulators::tag::variance,
                                         boost::accumulators::tag::skewness, boost::accumulators::tag::kurtosis>>>
        channel_stats(num_channels);

    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    for (uint i = 0; i < num_series; ++i) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            vec<Real> channel(series_len);
            dataset_ifs.read(reinterpret_cast<char *>(channel.data()), series_len * sizeof(Real));
            for (auto val : channel) channel_stats[c](val);
        }
    }

    return 0;
}
