#include "Summarization/Presence.hpp"

PresenceArray::PresenceArray(uint l_min, uint l_max, uint series_len, uint pos_per_env) {
    assert(l_min > 0 && l_max > l_min);

    if (pos_per_env == 0) pos_per_env = series_len;

    m_presences.resize(l_max + 2, 0);
    m_presence_sum = 0;
    for (uint l = l_max; l >= l_min; --l) {
        m_presences[l] = m_presences[l + 1] + std::min(pos_per_env, series_len - l + 1);
        m_presence_sum += m_presences[l];
    }
    for (uint l = l_min - 1; l > 0; --l) {
        m_presences[l] = m_presences[l + 1];
        m_presence_sum += m_presences[l];
    }
}

const vec<size_t> &PresenceArray::get_presences() const { return m_presences; }

size_t PresenceArray::get_presence_sum() const { return m_presence_sum; }
