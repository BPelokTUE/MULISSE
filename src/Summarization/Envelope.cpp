#include "Summarization/Envelope.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

Envelope::Envelope(vec<Real> lower, vec<Real> upper) : m_lower(std::move(lower)), m_upper(std::move(upper)) {}

void Envelope::resize(size_t new_size) {
    m_lower.resize(new_size);
    m_upper.resize(new_size);
}

vec<Real> Envelope::get_isax_input() const { return m_lower; }

// ----------------------------------------------- //
// --------------- ULISSE ENVELOPE --------------- //
// ----------------------------------------------- //

EnvelopeEntryGenerator::EnvelopeEntryGenerator(MtsNumChannelsT num_channels, bool normalized,
                                               const EnvelopeParams& uli_params, uint num_len_groups)
    : m_num_channels(num_channels), m_normalized(normalized), m_env_params(uli_params) {
    m_num_len_groups = num_len_groups;
}

vec<vec<IndexEntry<Envelope>>> EnvelopeEntryGenerator::get_entries(const vec<vec<Real>>& mts, uint series_ind) {
    uint series_len = U(mts[0].size());
    vec<vec<IndexEntry<Envelope>>> entries(m_num_len_groups);

    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto channel_envs_groups = m_normalized ? get_normalized_envelopes(mts[c]) : get_raw_envelopes(mts[c]);
        for (uint l = 0; l < m_num_len_groups; ++l) {
            auto& channel_envs = channel_envs_groups[l];
            for (uint i = 0; i < channel_envs.size(); ++i) {
                uint start_pos = i * m_env_params.m_pos_per_env;
                uint length = std::min(series_len + m_env_params.m_pos_per_env - 1, series_len - start_pos);

                if (c == 0) {
                    entries[l].resize(channel_envs.size());
                    entries[l][i].m_subs_info = {series_ind, start_pos, length};
                    entries[l][i].m_mts_summary.resize(m_num_channels);
                }
                entries[l][i].m_mts_summary[c] = std::move(channel_envs[i]);
            }
        }
    }

    return entries;
}

uint EnvelopeEntryGenerator::get_num_len_groups() const { return m_num_len_groups; }
