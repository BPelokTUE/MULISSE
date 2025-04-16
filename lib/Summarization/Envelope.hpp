#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Summarization/IndexEntry.hpp"

/**
 * @brief Envelope of a multivariate time series
 *
 * An envelope is a set of lower and upper bounds that summarizes subsequences of a multivariate time series
 */
struct Envelope : EntryData {
    /** @brief Lower bounds of the envelope */
    vec<Real> m_lower;
    /** @brief Upper bounds of the envelope */
    vec<Real> m_upper;

    Envelope() = default;

    /**
     * @brief Construct a new Envelope object
     * @param lower Lower bounds of the envelope
     * @param upper Upper bounds of the envelope
     */
    Envelope(vec<Real> lower, vec<Real> upper);

    inline size_t size() const override { return m_lower.size(); }

    inline void merge(const Envelope &other) {
        for (size_t i = 0; i < m_lower.size(); ++i) {
            m_lower[i] = std::min(m_lower[i], other.m_lower[i]);
            m_upper[i] = std::max(m_upper[i], other.m_upper[i]);
        }
    }

    void resize(size_t new_size) override;

    vec<Real> get_isax_input() const override;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_lower, m_upper);
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
    uint m_pos_per_env;
    uint m_segment_len;
    uint m_l_min;
    uint m_l_max;
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

        uint num_env = U((ts.size() - l_min + pos_per_env) / pos_per_env);
        vec<vec<Envelope>> envelope_groups = get_envelope_groups(num_env, l_min, l_max, segment_len);

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

        uint num_env = U((ts.size() - l_min + pos_per_env) / pos_per_env);

        vec<vec<Envelope>> envelope_groups = get_envelope_groups(num_env, l_min, l_max, segment_len);

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

                uint length_group = get_length_group(subs_len, l_min, l_max, m_num_len_groups);
                uint num_seg_in_subs = subs_len / segment_len;
                for (uint seg_ind = 0; seg_ind < num_seg_in_subs; ++seg_ind) {
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

    /**
     * @brief Declare the envelope groups with optimal size
     * @param num_env Number of envelopes per time series
     * @param l_min Minimum length of a query
     * @param l_max Maximum length of a query
     * @param segment_len Length of each segment
     */
    vec<vec<Envelope>> get_envelope_groups(const uint num_env, const uint l_min, const uint l_max,
                                           const uint segment_len) {
        vec<vec<Envelope>> envelope_groups(m_num_len_groups);
        for (uint lg_ind = 0; lg_ind < m_num_len_groups; ++lg_ind) {
            uint lg_l_max = l_min + (l_max - l_min) * (lg_ind + 1) / m_num_len_groups;
            SaxSegIndT segments_per_env_lg = static_cast<SaxSegIndT>((lg_l_max + segment_len - 1) / segment_len);

            envelope_groups[lg_ind].reserve(num_env);
            for (uint env_ind = 0; env_ind < num_env; ++env_ind)
                envelope_groups[lg_ind].emplace_back(vec<Real>(segments_per_env_lg, INF),
                                                     vec<Real>(segments_per_env_lg, -INF));
        }
        return envelope_groups;
    }

    /**
     * @brief Helper function to flip the values of envelope segments without data
     * @param envelope_groups Vector of envelope groups
     */
    inline void flip_env_infinities(vec<vec<Envelope>> &envelope_groups) {
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

    friend class EnvelopeTest;
};

#endif  // ENVELOPE_HPP
