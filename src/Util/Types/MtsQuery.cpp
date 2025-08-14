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

uint MtsQuery::get_query_len() const { return m_query_len; }

bool MtsQuery::is_channel_used(MtsNumChannelsT channel_ind) {
    if (channel_ind >= m_used_channels.size()) throw std::out_of_range("Channel index out of range");
    return m_used_channels[channel_ind];
}
