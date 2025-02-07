#include "Summarization/Envelope.hpp"
#include "Util/utilities.hpp"

size_t Envelope::size() const { return lower.size(); }

void Envelope::resize(size_t new_size) {
    lower.resize(new_size);
    upper.resize(new_size);
}

// ----------------------------------------------- //
// --------------- ULISSE ENVELOPE --------------- //
// ----------------------------------------------- //

void flip_env_infinities(vec<Envelope>& envelopes) {
    for (auto& envelope : envelopes) {
        for (SaxSegIndT s = 0; s < envelope.size(); ++s) {
            if (envelope.lower[s] > envelope.upper[s]) {
                envelope.lower[s] = -INF;
                envelope.upper[s] = INF;
            }
        }
    }
}

vec<Envelope> ulisse_envelope_raw(const vec<float>& ts, const UlisseEnvelopeParams& env_params) {
    auto [pos_per_env, segment_len, l_min, l_max] = env_params;

    uint segments_per_env = l_max / segment_len;
    uint num_env = (ts.size() - l_min + pos_per_env) / pos_per_env;
    vec<Envelope> envelopes(num_env, {vec<float>(segments_per_env, INF), vec<float>(segments_per_env, -INF)});

    float paa_acc = 0.0;

    for (int last_ind = 0; last_ind < ts.size(); ++last_ind) {
        paa_acc += ts[last_ind];
        uint subs_len = last_ind + 1;
        if (subs_len > segment_len) paa_acc -= ts[last_ind - segment_len];

        uint segments_in_subs = std::min(l_max, subs_len) / segment_len;

        float paa_val = paa_acc / segment_len;
        for (uint seg_ind = 0; seg_ind < segments_in_subs; ++seg_ind) {
            int first_ind = last_ind + 1 - (seg_ind + 1) * segment_len;
            if (ts.size() - first_ind >= l_min) {
                auto& envelope = envelopes[first_ind / pos_per_env];
                envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
            }
        }
    }
    flip_env_infinities(envelopes);
    return envelopes;
}

vec<Envelope> ulisse_envelope_normalized(const vec<float>& ts, const UlisseEnvelopeParams& env_params) {
    uint pos_per_env = env_params.pos_per_env, segment_len = env_params.segment_len, l_min = env_params.l_min,
         l_max = env_params.l_max;

    uint segments_per_env = l_max / segment_len;
    uint num_env = (ts.size() - l_min + pos_per_env) / pos_per_env;
    vec<Envelope> envelopes(num_env, {vec<float>(segments_per_env, INF), vec<float>(segments_per_env, -INF)});

    vec<float> sum_accs(ts.size() + 1, 0.0), sq_sum_accs(ts.size() + 1, 0.0);

    for (int last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum_accs[last_ind + 1] = sum_accs[last_ind] + ts[last_ind];
        sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + ts[last_ind] * ts[last_ind];

        int start_min = std::max(0, last_ind + 1 - static_cast<int>(l_max));
        int start_max = last_ind + 1 - static_cast<int>(l_min);

        for (int start = start_min; start <= start_max; ++start) {
            int subs_len = last_ind - start + 1;
            auto [mu, sigma] = calculate_mu_and_sigma(sum_accs[last_ind + 1] - sum_accs[start],
                                                      sq_sum_accs[last_ind + 1] - sq_sum_accs[start], subs_len);

            int num_seg_in_subs = subs_len / segment_len;
            for (int seg_ind = 0; seg_ind < num_seg_in_subs; ++seg_ind) {
                float paa_val =
                    (sum_accs[start + (seg_ind + 1) * segment_len] - sum_accs[start + seg_ind * segment_len]) /
                    segment_len;
                paa_val = (paa_val - mu) / sigma;

                auto& envelope = envelopes[start / pos_per_env];
                envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
            }
        }
    }
    flip_env_infinities(envelopes);
    return envelopes;
}

iSaxEnvelopeGenerator::iSaxEnvelopeGenerator(MtsNumChannelsT num_channels, bool normalized,
                                             const UlisseEnvelopeParams& uli_params)
    : m_num_channels(num_channels), m_normalized(normalized), m_uli_params(uli_params) {
    m_envelope_func = m_normalized ? ulisse_envelope_normalized : ulisse_envelope_raw;
}

vec<EnvelopeEntry> iSaxEnvelopeGenerator::get_entries(const vec<vec<float>>& mts, size_t series_ind) {
    uint series_len = mts[0].size();
    uint num_env = (series_len - m_uli_params.l_min + m_uli_params.pos_per_env) / m_uli_params.pos_per_env;
    vec<EnvelopeEntry> entries(num_env);

    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto channel_envs = m_envelope_func(mts[c], m_uli_params);
        for (size_t i = 0; i < channel_envs.size(); ++i) {
            size_t start_pos = i * m_uli_params.pos_per_env;

            if (c == 0) {
                entries[i].file_position = series_ind * m_num_channels * series_len + start_pos;
                entries[i].mts_envelope.resize(m_num_channels);
            }
            entries[i].mts_envelope[c] = std::move(channel_envs[i]);
        }
    }

    return entries;
}
