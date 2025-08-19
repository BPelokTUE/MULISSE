#include "Util/Types/MtsQuery.hpp"

MtsQuery::MtsQuery(const vec<vec<Real>> &&data) : MultivariateTimeSeries(std::move(data)) {
    m_query_len = 0;
    m_used_channels.resize(data.size(), false);

    for (MtsNumChannelsT c = 0; c < data.size(); ++c) {
        if (!data[c].empty()) {
            m_used_channels[c] = true;
            m_query_len = data[c].size();
        }
    }
}

bool MtsQuery::is_normalized() const { return m_normalized; }

const vec<bool> &MtsQuery::get_used_channels() const { return m_used_channels; }

bool MtsQuery::is_channel_used(MtsNumChannelsT channel_ind) const {
    if (channel_ind >= m_used_channels.size()) throw std::out_of_range("Channel index out of range");
    return m_used_channels[channel_ind];
}

uint MtsQuery::get_query_len() const { return m_query_len; }
