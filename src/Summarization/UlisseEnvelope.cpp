#include "Summarization/UlisseEnvelope.hpp"

UlisseEnvelope ulisse_envelope_raw(std::span<float> const& ts, unsigned ms_per_env, unsigned segment_len,
                                   unsigned l_min, unsigned l_max) {
    unsigned segments_per_env = l_max / segment_len;
    size_t ms_end = std::min(ts.size(), (size_t)(l_max + ms_per_env - 1));
    unsigned num_ms = std::min(ms_per_env, (unsigned)ms_end - l_min + 1);
    UlisseEnvelope envelope = {vec<float>(segments_per_env, std::numeric_limits<float>::max()),
                               vec<float>(segments_per_env, std::numeric_limits<float>::min())};

    float paa_acc = 0.0;
    vec<unsigned> segment_update_count(segments_per_env, 0);

    for (size_t i = 0; i < ms_end; ++i) {
        paa_acc += ts[i];
        unsigned subs_len = i + 1;
        if (subs_len > segment_len) paa_acc -= ts[i - segment_len];

        unsigned segments_in_subs = std::min(l_max, subs_len) / segment_len;

        for (unsigned seg_ind = 0; seg_ind < segments_in_subs; ++seg_ind) {
            if (segment_update_count[seg_ind] < num_ms) {
                ++segment_update_count[seg_ind];
                float paa_val = paa_acc / segment_len;
                envelope.first[seg_ind] = std::min(envelope.first[seg_ind], paa_val);
                envelope.second[seg_ind] = std::max(envelope.second[seg_ind], paa_val);
            }
        }
    }

    return envelope;
}

UlisseEnvelope ulisse_envelope_normalized(std::span<float> const& ts, unsigned ms_per_env, unsigned segment_len,
                                          unsigned l_min, unsigned l_max) {
    unsigned segments_per_env = l_max / segment_len;
    size_t ms_end = std::min(ts.size(), (size_t)(l_max + ms_per_env - 1));
    unsigned num_ms = std::min(ms_per_env, (unsigned)ms_end - l_min + 1);
    UlisseEnvelope envelope = {vec<float>(segments_per_env, std::numeric_limits<float>::max()),
                               vec<float>(segments_per_env, std::numeric_limits<float>::min())};

    vec<float> paa_accs(l_max + ms_per_env - segment_len, 0.0);

    unsigned n_seg = 0;
    float sum_acc = 0.0, sq_sum_acc = 0.0;

    for (size_t last_ind = 0; last_ind < ms_end; ++last_ind) {
        unsigned subs_len = last_ind + 1;

        if (subs_len > segment_len) {
            ++n_seg;
            paa_accs[n_seg] = paa_accs[n_seg - 1] - ts[last_ind - segment_len];
        }
        paa_accs[n_seg] += ts[last_ind];
        sum_acc += ts[last_ind];
        sq_sum_acc += ts[last_ind] * ts[last_ind];

        if (subs_len < l_min) continue;

        // If the subsequence is long enough, consider all its suffixes:
        // either one per master series in the envelope, or as many as possible, such that all of them are at least
        // `l_min` long. This also ensures that the same subsequence only appears in a single envelope, as the
        // envelopes are shifted by `ms_per_env`.
        unsigned num_suffix = std::min(ms_per_env, subs_len - l_min + 1);
        float sum_acc_tmp = sum_acc, sq_sum_acc_tmp = sq_sum_acc;

        for (unsigned i = 0; i < num_suffix; ++i) {
            size_t first_ind = i;
            unsigned suffix_len = last_ind - first_ind + 1;

            if (suffix_len <= l_max) {
                float mu = sum_acc_tmp / suffix_len, sigma = std::sqrt(sq_sum_acc_tmp / suffix_len - mu * mu);
                unsigned num_seg_in_suffix = suffix_len / segment_len;

                for (unsigned seg_ind = 0; seg_ind < num_seg_in_suffix; ++seg_ind) {
                    float paa_val = paa_accs[i + seg_ind * segment_len] / segment_len;
                    paa_val = (paa_val - mu) / sigma;
                    envelope.first[seg_ind] = std::min(envelope.first[seg_ind], paa_val);
                    envelope.second[seg_ind] = std::max(envelope.second[seg_ind], paa_val);
                }
            }

            sum_acc_tmp -= ts[first_ind];
            sq_sum_acc_tmp -= ts[first_ind] * ts[first_ind];
        }
    }

    return envelope;
}
