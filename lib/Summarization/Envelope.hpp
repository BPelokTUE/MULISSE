#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/SegmentationStrategy.hpp"

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
    const ISegmentationStrategy *m_segmentation_strategy;
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
    vec<vec<Envelope>> get_raw_envelopes(const vec<Real> &ts);

    /**
     * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization
     *
     * This function computes the ULISSE envelopes of subsequences of a time series between a
     * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    vec<vec<Envelope>> get_normalized_envelopes(const vec<Real> &ts);

    /**
     * @brief Declare the envelope groups with optimal size
     * @param num_env Number of envelopes per time series
     * @param l_min Minimum length of a query
     * @param l_max Maximum length of a query
     * @param segmentation_strategy Segmentation strategy to use
     */
    vec<vec<Envelope>> get_envelope_groups(const uint series_len, const uint pos_per_env, const uint l_min,
                                           const uint l_max, const ISegmentationStrategy *segmentation_strategy);

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
