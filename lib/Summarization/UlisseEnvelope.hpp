#ifndef ULISSE_ENVELOPE_HPP
#define ULISSE_ENVELOPE_HPP

#include "typedefs.hpp"

#include <span>

/**
 * @brief Compute the ULISSE envelopes of subsequences of a time series WITHOUT normalization.
 *
 * This function computes the ULISSE envelopes of subsequences of a time series between a
 * given range of start indices WITHOUT normalization. The envelopes are not discretized with iSAX.
 *
 * @param ts The (subsequence of the) time series.
 * @param ms_per_env The (max) number of master series in each envelope
 * @param segment_len The length of each PAA segment
 * @param l_min The minimum length of a subsequence
 * @param l_max The maximum length of a subsequence
 * @return A pair of two vectors, containing the upper and lower bounds of the subsequences respectively.
 */
UlisseEnvelope ulisse_envelope_raw(std::span<float> const& ts, unsigned ms_per_env, unsigned segment_len,
                                   unsigned l_min, unsigned l_max);

/**
 * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization.
 *
 * This function computes the ULISSE envelopes of subsequences of a time series between a
 * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
 *
 * @param ts The (subsequence of the) time series.
 * @param ms_per_env The (max) number of master series in each envelope
 * @param segment_len The length of each PAA segment
 * @param l_min The minimum length of a subsequence
 * @param l_max The maximum length of a subsequence
 * @return A pair of two vectors, containing the upper and lower bounds of the subsequences respectively.
 */
UlisseEnvelope ulisse_envelope_normalized(std::span<float> const& ts, unsigned ms_per_env, unsigned segment_len,
                                          unsigned l_min, unsigned l_max);

#endif  // ULISSE_ENVELOPE_HPP
