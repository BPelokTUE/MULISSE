#include "Util/Types/MultivariateTimeSeries.hpp"

MultivariateTimeSeries::MultivariateTimeSeries(const vec<vec<Real>> &&data) : m_data(data) {}

MtsNumChannelsT MultivariateTimeSeries::get_num_channels() const { return static_cast<MtsNumChannelsT>(m_data.size()); }

vec<Real> &MultivariateTimeSeries::operator[](size_t channel_idx) {
    if (channel_idx >= m_data.size()) throw std::out_of_range("Channel index out of range");
    return m_data[channel_idx];
}

const vec<Real> &MultivariateTimeSeries::operator[](size_t channel_idx) const {
    if (channel_idx >= m_data.size()) throw std::out_of_range("Channel index out of range");
    return m_data.at(channel_idx);
}
