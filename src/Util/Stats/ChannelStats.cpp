#include "Util/Stats/ChannelStats.hpp"

#include <cassert>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"

ChannelStats::ChannelStats(MtsNumChannelsT num_channels, Real mean, Real std)
    : m_means(num_channels, mean), m_stds(num_channels, std) {}

ChannelStats::ChannelStats(const vec<Real> &sums, const vec<Real> &sum_sqs, uint series_len, uint num_series) {
    assert(sums.size() == sum_sqs.size() && "Sums and sum squares must have the same number of channels");
    assert(series_len > 0 && "Series length must be greater than 0");
    assert(num_series > 0 && "Number of series must be greater than 0");

    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(sums.size());
    m_means.resize(num_channels);
    m_stds.resize(num_channels);

    size_t num_data_points = static_cast<size_t>(num_series) * static_cast<size_t>(series_len);
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        std::tie(m_means[c], m_stds[c]) = calculate_mu_and_sigma(sums[c], sum_sqs[c], num_data_points);
    }
}

template <typename Archive>
void ChannelStats::serialize(Archive &ar) {
    ar(cereal::make_nvp("means", m_means), cereal::make_nvp("stds", m_stds));
}
