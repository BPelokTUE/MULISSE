#include "Summarization/Envelope.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

Envelope::Envelope(vec<Real> lower, vec<Real> upper) {
    this->lower = std::move(lower);
    this->upper = std::move(upper);
}

size_t Envelope::size() const { return lower.size(); }

void Envelope::resize(size_t new_size) {
    lower.resize(new_size);
    upper.resize(new_size);
}

vec<Real> Envelope::get_isax_input() const { return lower; }

// ----------------------------------------------- //
// --------------- ULISSE ENVELOPE --------------- //
// ----------------------------------------------- //

void flip_env_infinities(vec<vec<Envelope>>& envelope_groups) {
    for (auto& envelope_group : envelope_groups) {
        for (auto& envelope : envelope_group) {
            for (SaxSegIndT s = 0; s < envelope.size(); ++s) {
                if (envelope.lower[s] > envelope.upper[s]) {
                    envelope.lower[s] = -INF;
                    envelope.upper[s] = INF;
                }
            }
        }
    }
}

EnvelopeEntryGenerator::EnvelopeEntryGenerator(MtsNumChannelsT num_channels, bool normalized,
                                               const EnvelopeParams& uli_params, uint num_length_groups)
    : m_num_channels(num_channels), m_normalized(normalized), m_env_params(uli_params) {
    m_num_length_groups = num_length_groups;
}

vec<vec<IndexEntry<Envelope>>> EnvelopeEntryGenerator::get_entries(const vec<vec<Real>>& mts, uint series_ind) {
    uint series_len = mts[0].size();
    uint num_env = (series_len - m_env_params.l_min + m_env_params.pos_per_env) / m_env_params.pos_per_env;
    vec<vec<IndexEntry<Envelope>>> entries(m_num_length_groups, vec<IndexEntry<Envelope>>(num_env));

    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto channel_envs_groups = m_normalized ? ulisse_envelope_normalized(mts[c]) : ulisse_envelope_raw(mts[c]);
        for (uint l = 0; l < m_num_length_groups; ++l) {
            auto& channel_envs = channel_envs_groups[l];
            for (uint i = 0; i < channel_envs.size(); ++i) {
                uint start_pos = i * m_env_params.pos_per_env;
                uint length = std::min(series_len + m_env_params.pos_per_env - 1, series_len - start_pos);

                if (c == 0) {
                    entries[l][i].subsequence_info = {series_ind, start_pos, length};
                    entries[l][i].mts_summary.resize(m_num_channels);
                }
                entries[l][i].mts_summary[c] = std::move(channel_envs[i]);
            }
        }
    }

    return entries;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::ulisse_envelope_raw(const vec<Real>& ts) {
    auto [pos_per_env, segment_len, l_min, l_max] = m_env_params;

    uint segments_per_env = l_max / segment_len;
    uint num_env = (ts.size() - l_min + pos_per_env) / pos_per_env;
    vec<vec<Envelope>> envelopes(m_num_length_groups, vec<Envelope>(num_env, {vec<Real>(segments_per_env, INF),
                                                                              vec<Real>(segments_per_env, -INF)}));

    Real paa_acc = 0.0;

    for (int last_ind = 0; last_ind < ts.size(); ++last_ind) {
        paa_acc += ts[last_ind];
        uint subs_len = last_ind + 1;
        if (subs_len > segment_len) paa_acc -= ts[last_ind - segment_len];

        uint segments_in_subs = std::min(l_max, subs_len) / segment_len;

        Real paa_val = paa_acc / segment_len;
        for (uint seg_ind = 0; seg_ind < segments_in_subs; ++seg_ind) {
            int first_ind = last_ind + 1 - (seg_ind + 1) * segment_len;
            uint length_group = get_length_group(subs_len, ts.size(), m_num_length_groups);
            if (ts.size() - first_ind >= l_min) {
                auto& envelope = envelopes[length_group][first_ind / pos_per_env];
                envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
            }
        }
    }
    flip_env_infinities(envelopes);
    return envelopes;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::ulisse_envelope_normalized(const vec<Real>& ts) {
    auto [pos_per_env, segment_len, l_min, l_max] = m_env_params;

    uint segments_per_env = l_max / segment_len;
    uint num_env = (ts.size() - l_min + pos_per_env) / pos_per_env;
    vec<vec<Envelope>> envelopes(m_num_length_groups, vec<Envelope>(num_env, {vec<Real>(segments_per_env, INF),
                                                                              vec<Real>(segments_per_env, -INF)}));

    vec<Real> sum_accs(ts.size() + 1, 0.0), sq_sum_accs(ts.size() + 1, 0.0);

    for (int last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum_accs[last_ind + 1] = sum_accs[last_ind] + ts[last_ind];
        sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + ts[last_ind] * ts[last_ind];

        int start_min = std::max(0, last_ind + 1 - static_cast<int>(l_max));
        int start_max = last_ind + 1 - static_cast<int>(l_min);

        for (int start = start_min; start <= start_max; ++start) {
            int subs_len = last_ind - start + 1;
            auto [mu, sigma] = calculate_mu_and_sigma(sum_accs[last_ind + 1] - sum_accs[start],
                                                      sq_sum_accs[last_ind + 1] - sq_sum_accs[start], subs_len);

            uint length_group = get_length_group(subs_len, ts.size(), m_num_length_groups);
            int num_seg_in_subs = subs_len / segment_len;
            for (int seg_ind = 0; seg_ind < num_seg_in_subs; ++seg_ind) {
                Real paa_val =
                    (sum_accs[start + (seg_ind + 1) * segment_len] - sum_accs[start + seg_ind * segment_len]) /
                    segment_len;
                paa_val = (paa_val - mu) / sigma;

                auto& envelope = envelopes[length_group][start / pos_per_env];
                envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
            }
        }
    }
    flip_env_infinities(envelopes);
    return envelopes;
}
