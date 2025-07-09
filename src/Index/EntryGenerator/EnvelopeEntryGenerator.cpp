#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/RunSettings/RunSettings.hpp"

using CHSS = ChannelSegmentationStrategyType;

EnvelopeEntryGenerator::EnvelopeEntryGenerator(bool normalized, uint pos_per_env, const LengthProperties &length_props,
                                               const ILengthGroupSegmentationStrategy *lg_segmentation_strategy,
                                               uint last_ind_step, uint first_ind_step)
    : m_normalized(normalized),
      m_pos_per_env(pos_per_env),
      m_length_props(length_props),
      m_lg_segmentation_strategy(lg_segmentation_strategy),
      m_last_ind_step(last_ind_step),
      m_first_ind_step(first_ind_step) {}

vec<vec<IndexEntry<Envelope>>> EnvelopeEntryGenerator::get_entries(const vec<vec<Real>> &mts, uint series_ind) {
    uint series_len = U(mts[0].size());
    vec<vec<IndexEntry<Envelope>>> entries(m_length_props.m_num_l_groups);

    for (MtsNumChannelsT c = 0; c < mts.size(); ++c) {
        auto channel_envs_groups = m_normalized ? get_normalized_envelopes(mts[c], c) : get_raw_envelopes(mts[c], c);
        for (uint l = 0; l < m_length_props.m_num_l_groups; ++l) {
            auto &channel_envs = channel_envs_groups[l];
            for (uint i = 0; i < channel_envs.size(); ++i) {
                uint start_pos = i * m_pos_per_env;
                uint num_start_pos = std::min(m_pos_per_env, series_len - start_pos);

                if (c == 0) {
                    entries[l].resize(channel_envs.size());
                    entries[l][i].m_subs_info = {series_ind, start_pos, num_start_pos};
                    entries[l][i].m_mts_summary.resize(mts.size());
                }
                entries[l][i].m_mts_summary[c] = std::move(channel_envs[i]);
            }
        }
    }
    return entries;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::get_raw_envelopes(const vec<Real> &ts, MtsNumChannelsT ch_ind) {
    if (m_length_props.m_num_l_groups != 1) {
        throw std::runtime_error("Length-based grouping is not supported for raw envelopes");
    }

    auto [use_lg, l_min, l_max, l_per_group, num_l_groups] = m_length_props;

    auto ch_segmentation_strategy = m_lg_segmentation_strategy->get_const_ch_segmentation_strategy(0);
    auto segmentation_strategy = ch_segmentation_strategy->get_const_segmentation_strategy(ch_ind);
    if (m_lg_segmentation_strategy->get_type() != SINGLE || segmentation_strategy->get_type() != UNIFORM) {
        throw std::runtime_error(
            "get_raw_envelopes is only supported for SingleLGSegmentationStrategy + UniformSegmentationStrategy");
    }
    uint segment_len = segmentation_strategy->get_segment_len(0);

    vec<vec<Envelope>> envelope_groups =
        get_envelope_groups(U(ts.size()), m_pos_per_env, l_min, l_max, m_lg_segmentation_strategy, ch_ind);

    Real paa_acc = 0.0, segment_len_r = R(segment_len);

    for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
        paa_acc += ts[last_ind];
        uint prefix_len = last_ind + 1;
        if (prefix_len > segment_len) paa_acc -= ts[last_ind - segment_len];

        if (last_ind % m_last_ind_step != 0) continue;

        uint segments_in_subs = std::min(l_max, prefix_len) / segment_len;

        Real paa_val = paa_acc / segment_len_r;
        for (uint seg_ind = 0; seg_ind < segments_in_subs; ++seg_ind) {
            uint first_ind = last_ind + 1 - (seg_ind + 1) * segment_len;
            if (ts.size() - first_ind >= l_min) {
                auto &envelope = envelope_groups[0][first_ind / m_pos_per_env];
                envelope.m_lower[seg_ind] = std::min(envelope.m_lower[seg_ind], paa_val);
                envelope.m_upper[seg_ind] = std::max(envelope.m_upper[seg_ind], paa_val);
            }
        }
    }

    flip_env_infinities(envelope_groups);

    // TODO: Solve this more elegantly
    // Normalize the envelope segments with the dataset statistics. Note: this is not subsequence Z-normalization, it is
    // needed for EquiprobableBreakpointStrategy
    auto [ch_mean, ch_std] = RunSettings::get_instance().get_channel_mean_and_std(ch_ind);
    for (auto &envelope_group : envelope_groups) {
        for (auto &envelope : envelope_group) {
            for (SaxSegIndT s = 0; s < envelope.m_lower.size(); ++s) {
                envelope.m_lower[s] = (envelope.m_lower[s] - ch_mean) / ch_std;
                envelope.m_upper[s] = (envelope.m_upper[s] - ch_mean) / ch_std;
            }
        }
    }
    return envelope_groups;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::get_normalized_envelopes(const vec<Real> &ts, MtsNumChannelsT ch_ind) {
    auto [use_lg, l_min, l_max, l_per_group, num_l_groups] = m_length_props;

    vec<vec<Envelope>> envelope_groups =
        get_envelope_groups(U(ts.size()), m_pos_per_env, l_min, l_max, m_lg_segmentation_strategy, ch_ind);

    vec<Real> sum_accs(ts.size() + 1, 0.0), sq_sum_accs(ts.size() + 1, 0.0);

    for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum_accs[last_ind + 1] = sum_accs[last_ind] + ts[last_ind];
        sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + ts[last_ind] * ts[last_ind];

        if (last_ind % m_last_ind_step != 0) continue;

        uint start_min = U(std::max(0, static_cast<int>(last_ind + 1 - l_max)));
        int start_max = static_cast<int>(last_ind + 1 - l_min);

        for (uint first_ind = start_min; static_cast<int>(first_ind) <= start_max; first_ind += m_first_ind_step) {
            uint subs_len = last_ind - first_ind + 1;
            auto [mu, sigma] = calculate_mu_and_sigma(sum_accs[last_ind + 1] - sum_accs[first_ind],
                                                      sq_sum_accs[last_ind + 1] - sq_sum_accs[first_ind], subs_len);

            uint segment_len_sum = 0;
            uint length_group = m_length_props.get_length_group(subs_len);
            auto segmentation_strategy = m_lg_segmentation_strategy->get_const_ch_segmentation_strategy(length_group)
                                             ->get_const_segmentation_strategy(ch_ind);

            SaxSegIndT num_segments = segmentation_strategy->get_num_segments(subs_len);
            for (SaxSegIndT seg_ind = 0; seg_ind < num_segments; ++seg_ind) {
                uint segment_len = segmentation_strategy->get_segment_len(seg_ind);
                Real paa_val =
                    (sum_accs[first_ind + segment_len_sum + segment_len] - sum_accs[first_ind + segment_len_sum]) /
                    R(segment_len);
                segment_len_sum += segment_len;
                paa_val = (paa_val - mu) / sigma;

                auto &envelope = envelope_groups[length_group][first_ind / m_pos_per_env];
                envelope.m_lower[seg_ind] = std::min(envelope.m_lower[seg_ind], paa_val);
                envelope.m_upper[seg_ind] = std::max(envelope.m_upper[seg_ind], paa_val);
            }
        }
    }
    flip_env_infinities(envelope_groups);
    return envelope_groups;
}

vec<vec<Envelope>> EnvelopeEntryGenerator::get_envelope_groups(
    const uint series_len, const uint pos_per_env, const uint l_min, const uint l_max,
    const ILengthGroupSegmentationStrategy *lg_segmentation_strategy, MtsNumChannelsT ch_ind) {
    vec<vec<Envelope>> envelope_groups(m_length_props.m_num_l_groups);
    for (uint lg_ind = 0; lg_ind < m_length_props.m_num_l_groups; ++lg_ind) {
        uint lg_l_min = m_length_props.get_lg_l_min(lg_ind);
        uint lg_l_max = m_length_props.get_lg_l_max(lg_ind);
        uint num_env = U((series_len - lg_l_min + pos_per_env) / pos_per_env);

        SaxSegIndT segments_per_env_lg = lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind)
                                             ->get_const_segmentation_strategy(ch_ind)
                                             ->get_num_segments(lg_l_max);

        envelope_groups[lg_ind].reserve(num_env);
        for (uint env_ind = 0; env_ind < num_env; ++env_ind)
            envelope_groups[lg_ind].emplace_back(vec<Real>(segments_per_env_lg, INF),
                                                 vec<Real>(segments_per_env_lg, -INF));
    }
    return envelope_groups;
}

void EnvelopeEntryGenerator::flip_env_infinities(vec<vec<Envelope>> &envelope_groups) {
    for (auto &envelope_group : envelope_groups) {
        for (auto &envelope : envelope_group) {
            for (SaxSegIndT s = 0; s < envelope.m_lower.size(); ++s) {
                if (envelope.m_lower[s] > envelope.m_upper[s]) {
                    envelope.m_lower[s] = -INF;
                    envelope.m_upper[s] = INF;
                }
            }
        }
    }
}
