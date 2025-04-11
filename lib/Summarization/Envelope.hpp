#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"

/**
 * @brief Envelope of a multivariate time series
 *
 * An envelope is a set of lower and upper bounds that summarizes subsequences of a multivariate time series
 */
struct Envelope : EntryData {
    /** @brief Lower bounds of the envelope */
    vec<Real> lower;
    /** @brief Upper bounds of the envelope */
    vec<Real> upper;

    Envelope(vec<Real> lower, vec<Real> upper);

    Envelope() = default;

    size_t size() const override;

    void resize(size_t new_size) override;

    vec<Real> get_isax_input() const override;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(lower, upper);
    }
};

// ----------------------------------------------- //
// --------------- ULISSE ENVELOPE --------------- //
// ----------------------------------------------- //

/**
 * @brief Parameters for the ULISSE Envelope computation
 *
 * This struct contains the parameters needed to compute the ULISSE envelopes of (a subsequences of) a time series
 *
 * @param pos_per_env The (max) number of master series in each envelope
 * @param segment_len The length of each PAA segment
 * @param l_min The minimum length of a subsequence
 * @param l_max The maximum length of a subsequence
 */
struct EnvelopeParams {
    uint pos_per_env;
    uint segment_len;
    uint l_min;
    uint l_max;
};

/** @brief Envelope generator for iSAX (ULISSE) envelopes */
class EnvelopeEntryGenerator : public IEntryGenerator<Envelope> {
   public:
    /**
     * @brief Construct a new EnvelopeEntryGenerator object
     * @param num_channels Number of channels in each series
     * @param normalized Whether to normalize the subsequences
     * @param env_params Parameters for the ULISSE envelope computation
     * @param num_length_groups Number of length groups
     */
    EnvelopeEntryGenerator(MtsNumChannelsT num_channels, bool normalized, const EnvelopeParams &env_params,
                           uint num_length_groups = 1);

    vec<vec<IndexEntry<Envelope>>> get_entries(const vec<vec<Real>> &mts, uint series_ind) override;

    uint get_num_len_groups() const override;

   private:
    bool m_normalized;
    MtsNumChannelsT m_num_channels;
    uint m_num_len_groups;
    EnvelopeParams m_env_params;

    /**
     * @brief Compute the ULISSE envelopes of subsequences of a time series WITHOUT normalization
     *
     * This function computes the ULISSE envelopes of subsequences of a time series between a
     * given range of start indices WITHOUT normalization. The envelopes are not discretized with iSAX. In the case of
     * raw envelopes, overlapping segments of subsequences are identical, and therefore length-based grouping is
     * useless. Nevertheless, if requested, the envelopes are repeated for each length group.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    inline vec<vec<Envelope>> get_raw_envelopes(const vec<Real> &ts) {
        auto [pos_per_env, segment_len, l_min, l_max] = m_env_params;

        uint segments_per_env = l_max / segment_len;
        uint num_env = static_cast<uint>((ts.size() - l_min + pos_per_env) / pos_per_env);
        vec<vec<Envelope>> envelope_groups(
            m_num_len_groups,
            vec<Envelope>(num_env, {vec<Real>(segments_per_env, INF), vec<Real>(segments_per_env, -INF)}));

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
                    auto &envelope = envelope_groups[0][first_ind / pos_per_env];
                    envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                    envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
                }
            }
        }

        flip_env_infinities(envelope_groups);
        return envelope_groups;
    }

    /**
     * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization
     *
     * This function computes the ULISSE envelopes of subsequences of a time series between a
     * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    inline vec<vec<Envelope>> get_normalized_envelopes(const vec<Real> &ts) {
        auto [pos_per_env, segment_len, l_min, l_max] = m_env_params;

        uint segments_per_env = l_max / segment_len;
        uint num_env = static_cast<uint>((ts.size() - l_min + pos_per_env) / pos_per_env);
        vec<vec<Envelope>> envelopes(m_num_len_groups, vec<Envelope>(num_env, {vec<Real>(segments_per_env, INF),
                                                                               vec<Real>(segments_per_env, -INF)}));

        vec<Real> sum_accs(ts.size() + 1, 0.0), sq_sum_accs(ts.size() + 1, 0.0);

        for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
            sum_accs[last_ind + 1] = sum_accs[last_ind] + ts[last_ind];
            sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + ts[last_ind] * ts[last_ind];

            uint start_min = static_cast<uint>(std::max(0, static_cast<int>(last_ind + 1 - l_max)));
            int start_max = static_cast<int>(last_ind + 1 - l_min);

            for (uint start = start_min; static_cast<int>(start) <= start_max; ++start) {
                uint subs_len = last_ind - start + 1;
                auto [mu, sigma] = calculate_mu_and_sigma(sum_accs[last_ind + 1] - sum_accs[start],
                                                          sq_sum_accs[last_ind + 1] - sq_sum_accs[start], subs_len);

                uint length_group = get_length_group(subs_len, l_min, l_max, m_num_len_groups);
                uint num_seg_in_subs = subs_len / segment_len;
                for (uint seg_ind = 0; seg_ind < num_seg_in_subs; ++seg_ind) {
                    Real paa_val =
                        (sum_accs[start + (seg_ind + 1) * segment_len] - sum_accs[start + seg_ind * segment_len]) /
                        R(segment_len);
                    paa_val = (paa_val - mu) / sigma;

                    auto &envelope = envelopes[length_group][start / pos_per_env];
                    envelope.lower[seg_ind] = std::min(envelope.lower[seg_ind], paa_val);
                    envelope.upper[seg_ind] = std::max(envelope.upper[seg_ind], paa_val);
                }
            }
        }
        flip_env_infinities(envelopes);
        return envelopes;
    }

    /**
     * @brief Helper function to flip the values of envelope segments without data
     * @param envelope_groups Vector of envelope groups
     */
    void flip_env_infinities(vec<vec<Envelope>> &envelope_groups) {
        for (auto &envelope_group : envelope_groups) {
            for (auto &envelope : envelope_group) {
                for (SaxSegIndT s = 0; s < envelope.size(); ++s) {
                    if (envelope.lower[s] > envelope.upper[s]) {
                        envelope.lower[s] = -INF;
                        envelope.upper[s] = INF;
                    }
                }
            }
        }
    }

    friend class EnvelopeTest;
};

#endif  // ENVELOPE_HPP
