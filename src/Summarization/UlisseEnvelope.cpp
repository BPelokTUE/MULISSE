#include "Summarization/UlisseEnvelope.hpp"

vec<Envelope> ulisse_envelope_raw(const vec<float>& ts, const UlisseEnvelopeParams& env_params) {
    auto [ms_per_env, segment_len, l_min, l_max] = env_params;

    unsigned segments_per_env = l_max / segment_len;
    unsigned num_env = (ts.size() - l_min + ms_per_env) / ms_per_env;
    vec<Envelope> envelopes(num_env, {vec<float>(segments_per_env, INF), vec<float>(segments_per_env, NEG_INF)});

    float paa_acc = 0.0;

    for (size_t last_ind = 0; last_ind < ts.size(); ++last_ind) {
        paa_acc += ts[last_ind];
        unsigned subs_len = last_ind + 1;
        if (subs_len > segment_len) paa_acc -= ts[last_ind - segment_len];

        unsigned segments_in_subs = std::min(l_max, subs_len) / segment_len;

        float paa_val = paa_acc / segment_len;
        for (unsigned seg_ind = 0; seg_ind < segments_in_subs; ++seg_ind) {
            size_t first_ind = last_ind - (seg_ind + 1) * segment_len + 1;
            if (ts.size() - first_ind >= l_min) {
                auto& envelope = envelopes[first_ind / ms_per_env];
                envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
            }
        }
    }

    return envelopes;
}

vec<Envelope> ulisse_envelope_normalized(const vec<float>& ts, const UlisseEnvelopeParams& env_params) {
    auto [ms_per_env, segment_len, l_min, l_max] = env_params;

    unsigned segments_per_env = l_max / segment_len;
    unsigned num_env = (ts.size() - l_min + ms_per_env) / ms_per_env;
    vec<Envelope> envelopes(num_env, {vec<float>(segments_per_env, INF), vec<float>(segments_per_env, NEG_INF)});

    vec<float> paa_accs(ts.size() - segment_len + 1, 0.0);
    unsigned n_seg = 0;
    float sum_acc = 0.0, sq_sum_acc = 0.0;

    for (unsigned last_ind = 0; last_ind < ts.size(); ++last_ind) {
        unsigned prefix_len = last_ind + 1;

        if (prefix_len > segment_len) {
            ++n_seg;
            paa_accs[n_seg] = paa_accs[n_seg - 1] - ts[last_ind - segment_len];
        }
        paa_accs[n_seg] += ts[last_ind];
        sum_acc += ts[last_ind];
        sq_sum_acc += ts[last_ind] * ts[last_ind];

        if (prefix_len < l_min) continue;

        // If the subsequence is long enough, consider all its suffixes which are also prefixes of one of the master
        // series (i.e. suffixes that start at one of the allowed positions):
        // either one per master series in the envelope, or as many as possible, such that all of them are at
        // least `l_min` long. This also ensures that the same subsequence only appears in a single envelope, as the
        // envelopes are shifted by `ms_per_env`.
        unsigned num_suffix = prefix_len - l_min + 1;
        float sum_acc_tmp = sum_acc, sq_sum_acc_tmp = sq_sum_acc;

        for (unsigned first_ind = 0; first_ind < num_suffix; ++first_ind) {
            unsigned suffix_len = last_ind - first_ind + 1;
            if (suffix_len <= l_max) {
                // TODO: get rid of square root
                float mu = sum_acc_tmp / suffix_len, sigma = std::sqrt(sq_sum_acc_tmp / suffix_len - mu * mu);
                unsigned num_seg_in_suffix = suffix_len / segment_len;

                for (unsigned seg_ind = 0; seg_ind < num_seg_in_suffix; ++seg_ind) {
                    float paa_val = paa_accs[first_ind + seg_ind * segment_len] / segment_len;
                    paa_val = (paa_val - mu) / sigma;
                    // select appropriate envelope
                    auto& envelope = envelopes[first_ind / ms_per_env];
                    envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                    envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
                }
            }
            sum_acc_tmp -= ts[first_ind];
            sq_sum_acc_tmp -= ts[first_ind] * ts[first_ind];
        }
    }
    return envelopes;
}
