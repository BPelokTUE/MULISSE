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

// EnvelopeEntryGenerator

EnvelopeEntryGenerator::EnvelopeEntryGenerator(MtsNumChannelsT num_channels, bool normalized,
                                               const EnvelopeParams &uli_params, uint num_len_groups)
    : m_num_channels(num_channels), m_normalized(normalized), m_env_params(uli_params) {
    m_num_len_groups = num_len_groups;
}

vec<vec<IndexEntry<Envelope>>> EnvelopeEntryGenerator::get_entries(const vec<vec<Real>> &mts, uint series_ind) {
    uint series_len = U(mts[0].size());
    vec<vec<IndexEntry<Envelope>>> entries(m_num_len_groups);

    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto channel_envs_groups = m_normalized ? get_normalized_envelopes(mts[c]) : get_raw_envelopes(mts[c]);
        for (uint l = 0; l < m_num_len_groups; ++l) {
            auto &channel_envs = channel_envs_groups[l];
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

vec<vec<Envelope>> EnvelopeEntryGenerator::get_raw_envelopes(const vec<Real> &ts) {
    auto [pos_per_env, segmentation_strategy, l_min, l_max] = m_env_params;

    if (segmentation_strategy->get_type() != UNIFORM) {
        throw std::runtime_error("get_raw_envelopes is only supported for UniformSegmentationStrategy");
    }
    uint segment_len = segmentation_strategy->get_segment_len(0);

    vec<vec<Envelope>> envelope_groups =
        get_envelope_groups(U(ts.size()), pos_per_env, l_min, l_max, segmentation_strategy);

    Real paa_acc = 0.0, segment_len_r = R(segment_len);

    for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
        paa_acc += ts[last_ind];
        uint prefix_len = last_ind + 1;
        if (prefix_len > segment_len) paa_acc -= ts[last_ind - segment_len];

        uint segments_in_subs = std::min(l_max, prefix_len) / segment_len;

        Real paa_val = paa_acc / segment_len_r;
        for (uint seg_ind = 0; seg_ind < segments_in_subs; ++seg_ind) {
            uint first_ind = last_ind + 1 - (seg_ind + 1) * segment_len;
            if (ts.size() - first_ind >= l_min) {
                auto &envelope = envelope_groups[m_num_len_groups - 1][first_ind / pos_per_env];
                envelope.m_lower[seg_ind] = std::min(envelope.m_lower[seg_ind], paa_val);
                envelope.m_upper[seg_ind] = std::max(envelope.m_upper[seg_ind], paa_val);
            }
        }
    }

    for (uint lg_ind = 0; lg_ind < m_num_len_groups - 1; ++lg_ind)
        for (uint seg_ind = 0; seg_ind < envelope_groups[lg_ind].size(); ++seg_ind)
            envelope_groups[lg_ind][seg_ind] = envelope_groups[m_num_len_groups - 1][seg_ind];

    flip_env_infinities(envelope_groups);
    return envelope_groups;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::get_normalized_envelopes(const vec<Real> &ts) {
    auto &RS = RunSettings::get_instance();
    auto [pos_per_env, segmentation_strategy, l_min, l_max] = m_env_params;

    vec<vec<Envelope>> envelope_groups =
        get_envelope_groups(U(ts.size()), pos_per_env, l_min, l_max, segmentation_strategy);

    vec<Real> sum_accs(ts.size() + 1, 0.0), sq_sum_accs(ts.size() + 1, 0.0);

    for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum_accs[last_ind + 1] = sum_accs[last_ind] + ts[last_ind];
        sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + ts[last_ind] * ts[last_ind];

        uint start_min = U(std::max(0, static_cast<int>(last_ind + 1 - l_max)));
        int start_max = static_cast<int>(last_ind + 1 - l_min);

        for (uint start = start_min; static_cast<int>(start) <= start_max; ++start) {
            uint subs_len = last_ind - start + 1;
            auto [mu, sigma] = calculate_mu_and_sigma(sum_accs[last_ind + 1] - sum_accs[start],
                                                      sq_sum_accs[last_ind + 1] - sq_sum_accs[start], subs_len);

            uint length_group = RS.get_length_group(subs_len);
            uint lg_l_max = RS.get_lg_l_max(length_group);

            SaxSegIndT num_segments = segmentation_strategy->get_num_segments(lg_l_max);
            for (SaxSegIndT seg_ind = 0; seg_ind < num_segments; ++seg_ind) {
                uint segment_len = segmentation_strategy->get_segment_len(seg_ind);
                Real paa_val =
                    (sum_accs[start + (seg_ind + 1) * segment_len] - sum_accs[start + seg_ind * segment_len]) /
                    R(segment_len);
                paa_val = (paa_val - mu) / sigma;

                auto &envelope = envelope_groups[length_group][start / pos_per_env];
                envelope.m_lower[seg_ind] = std::min(envelope.m_lower[seg_ind], paa_val);
                envelope.m_upper[seg_ind] = std::max(envelope.m_upper[seg_ind], paa_val);
            }
        }
    }
    flip_env_infinities(envelope_groups);
    return envelope_groups;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::get_envelope_groups(const uint series_len, const uint pos_per_env,
                                                               const uint l_min, const uint l_max,
                                                               const ISegmentationStrategy *segmentation_strategy) {
    auto &RS = RunSettings::get_instance();

    vec<vec<Envelope>> envelope_groups(m_num_len_groups);
    for (uint lg_ind = 0; lg_ind < m_num_len_groups; ++lg_ind) {
        uint lg_l_min = RS.get_lg_l_min(lg_ind);  // l_min + (l_max - l_min) * lg_ind / m_num_len_groups;
        uint lg_l_max = RS.get_lg_l_max(lg_ind);
        uint num_env = U((series_len - lg_l_min + pos_per_env) / pos_per_env);
        // CHECK: this might need to be rounded up, but probably not
        SaxSegIndT segments_per_env_lg = segmentation_strategy->get_num_segments(lg_l_max);

        envelope_groups[lg_ind].reserve(num_env);
        for (uint env_ind = 0; env_ind < num_env; ++env_ind)
            envelope_groups[lg_ind].emplace_back(vec<Real>(segments_per_env_lg, INF),
                                                 vec<Real>(segments_per_env_lg, -INF));
    }
    return envelope_groups;
}
