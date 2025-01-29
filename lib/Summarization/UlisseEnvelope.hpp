#ifndef ULISSE_ENVELOPE_HPP
#define ULISSE_ENVELOPE_HPP

#include <span>

#include "typedefs.hpp"
#include "Summarization/Envelope.hpp"

/**
 * @brief Parameters for the ULISSE envelope computation.
 *
 * This struct contains the parameters needed to compute the ULISSE envelopes of (a subsequences of) a time series.
 *
 * @param pos_per_env The (max) number of master series in each envelope
 * @param segment_len The length of each PAA segment
 * @param l_min The minimum length of a subsequence
 * @param l_max The maximum length of a subsequence
 */
struct UlisseEnvelopeParams {
    unsigned pos_per_env;
    unsigned segment_len;
    unsigned l_min;
    unsigned l_max;
};

/**
 * @brief Compute the ULISSE envelopes of subsequences of a time series WITHOUT normalization.
 *
 * This function computes the ULISSE envelopes of subsequences of a time series between a
 * given range of start indices WITHOUT normalization. The envelopes are not discretized with iSAX.
 *
 * @param ts The (subsequence of the) time series.
 * @param env_params The parameters for the envelope computation.
 * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively.
 */
vec<Envelope> ulisse_envelope_raw(const vec<float>& ts, const UlisseEnvelopeParams& env_params);

/**
 * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization.
 *
 * This function computes the ULISSE envelopes of subsequences of a time series between a
 * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
 *
 * @param ts The (subsequence of the) time series.
 * @param env_params The parameters for the envelope computation.
 * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively.
 */
Envelope ulisse_envelope_normalized(std::span<const float> const& ts, const UlisseEnvelopeParams& env_params);

#endif  // ULISSE_ENVELOPE_HPP
