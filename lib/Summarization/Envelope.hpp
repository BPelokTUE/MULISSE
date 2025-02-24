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
    vec<float> lower;
    /** @brief Upper bounds of the envelope */
    vec<float> upper;

    /**
     * @brief Get the size (number of entries) of the envelope
     *
     * @return The size of the envelope
     * */
    size_t size() const;

    /**
     * @brief Resize the envelope
     *
     * @param new_size The new size of the envelope
     * */
    void resize(size_t new_size);

    vec<float> get_isax_input() override;
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
struct UlisseEnvelopeParams {
    uint pos_per_env;
    uint segment_len;
    uint l_min;
    uint l_max;
};

/**
 * @brief Compute the ULISSE envelopes of subsequences of a time series WITHOUT normalization
 *
 * This function computes the ULISSE envelopes of subsequences of a time series between a
 * given range of start indices WITHOUT normalization. The envelopes are not discretized with iSAX.
 *
 * @param ts The (subsequence of the) univariate time series / channel
 * @param env_params The parameters for the envelope computation
 * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
 */
vec<Envelope> ulisse_envelope_raw(const vec<float> &ts, const UlisseEnvelopeParams &env_params);

/**
 * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization
 *
 * This function computes the ULISSE envelopes of subsequences of a time series between a
 * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
 *
 * @param ts The (subsequence of the) univariate time series / channel
 * @param env_params The parameters for the envelope computation
 * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
 */
vec<Envelope> ulisse_envelope_normalized(const vec<float> &ts, const UlisseEnvelopeParams &env_params);

/** @brief Envelope generator for iSAX (ULISSE) envelopes */
class iSaxEnvelopeGenerator : public IEntryGenerator<Envelope> {
   public:
    /**
     * @brief Construct a new iSaxEnvelopeGenerator object
     * @param opts Indexing options
     */
    iSaxEnvelopeGenerator(MtsNumChannelsT num_channels, bool normalized, const UlisseEnvelopeParams &uli_params);

    vec<IndexEntry<Envelope>> get_entries(const vec<vec<float>> &mts, uint series_ind) override;

   private:
    MtsNumChannelsT m_num_channels;
    bool m_normalized;
    UlisseEnvelopeParams m_uli_params;
    vec<Envelope> (*m_envelope_func)(const vec<float> &, const UlisseEnvelopeParams &);
};

#endif  // ENVELOPE_HPP
