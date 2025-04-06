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
     * given range of start indices WITHOUT normalization. The envelopes are not discretized with iSAX.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    inline vec<vec<Envelope>> ulisse_envelope_raw(const vec<Real> &ts);

    /**
     * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization
     *
     * This function computes the ULISSE envelopes of subsequences of a time series between a
     * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    inline vec<vec<Envelope>> ulisse_envelope_normalized(const vec<Real> &ts);
};

#endif  // ENVELOPE_HPP
