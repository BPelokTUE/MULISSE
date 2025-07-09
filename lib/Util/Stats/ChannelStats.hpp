#ifndef UTIL_STATS_CHANNELSTATS_HPP
#define UTIL_STATS_CHANNELSTATS_HPP

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

class ChannelStats {
    friend class RunSettings;

   public:
    /**
     * @brief Construct a new ChannelStats object, setting hardcoded mean and std values for all channels
     * @param num_channels Number of channels, defaults to 0 (no stats)
     * @param mean Mean value for the channels, defaults to 0
     * @param std Standard deviation for the channels, defaults to 1
     */
    ChannelStats(MtsNumChannelsT num_channels = 0, Real mean = R(0.0), Real std = R(1.0));

    /**
     * @brief Construct a new ChannelStats object from sums and sums of squares
     * @param sums The sum of the values in each channel.
     * @param sum_sqs The sum of the squares of the values in each channel.
     * @param series_len The length of each time series.
     * @param num_series The number of time series.
     */
    ChannelStats(const vec<Real> &sums, const vec<Real> &sum_sqs, uint series_len, uint num_series);

    /**
     * @brief Save the dataset stats to a `.json` file.
     * @param filename The name of the file. Should be a `.json` file.
     */
    void save(const str &filename) const;

    /**
     * @brief Load the dataset stats from a `.json` file.
     * @param filename The name of the file. Should be a `.json` file.
     */
    void load(const str &filename);

   private:
    vec<Real> m_means, m_stds;
};

#endif  // UTIL_STATS_CHANNELSTATS_HPP
