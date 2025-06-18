#ifndef INDEX_ENTRYGENERATOR_ENVELOPEENTRYGENERATOR_HPP
#define INDEX_ENTRYGENERATOR_ENVELOPEENTRYGENERATOR_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Util/RunSettings/LengthProperties.hpp"

/** @brief Envelope generator for MULISSE envelopes */
class EnvelopeEntryGenerator : public IEntryGenerator<Envelope> {
   public:
    /**
     * @brief Construct a new EnvelopeEntryGenerator object
     * @param normalized Whether to normalize the subsequences
     * @param pos_per_env Positions per envelope
     * @param length_props Length properties
     * @param lg_segmentation_strategy ILengthGroupSegmentationStrategy to use for length-based grouping
     * @param last_ind_step Optional for creating dummy envelopes, defaults to 1. Other values will result in envelopes
     * that do not guarantee exact results when used for min-dist calculation.
     * @param first_ind_step Optional for creating dummy envelopes, defaults to 1. Other values will result in envelopes
     * that do not guarantee exact results when used for min-dist calculation in the normalized case.
     */
    EnvelopeEntryGenerator(bool normalized, uint pos_per_env, const LengthProperties &length_props,
                           const ILengthGroupSegmentationStrategy *lg_segmentation_strategy, uint last_ind_step = 1,
                           uint first_ind_step = 1);

    vec<vec<IndexEntry<Envelope>>> get_entries(const vec<vec<Real>> &mts, uint series_ind) override;

   private:
    bool m_normalized;
    uint m_pos_per_env, m_last_ind_step, m_first_ind_step;
    LengthProperties m_length_props;
    const ILengthGroupSegmentationStrategy *m_lg_segmentation_strategy;

    /**
     * @brief Compute the ULISSE envelopes of subsequences of a time series WITHOUT normalization
     *
     * This function computes the ULISSE envelopes of subsequences of a time series between a
     * given range of start indices WITHOUT normalization. The envelopes are not discretized with iSAX. In the case of
     * raw envelopes, overlapping segments of subsequences are identical, and therefore length-based grouping is
     * useless. Nevertheless, if requested, the envelopes are repeated for each length group.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @param ch_ind The channel index
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    vec<vec<Envelope>> get_raw_envelopes(const vec<Real> &ts, MtsNumChannelsT ch_ind);

    /**
     * @brief Compute the ULISSE envelopes of subsequences of a time series WITH normalization
     *
     * This function computes the ULISSE envelopes of subsequences of a time series between a
     * given range of start indices WITH normalization. The envelopes are not discretized with iSAX.
     *
     * @param ts The (subsequence of the) univariate time series / channel
     * @param ch_ind The channel index
     * @return Vector of vector pairs containing the upper and lower bounds of the subsequences respectively
     */
    vec<vec<Envelope>> get_normalized_envelopes(const vec<Real> &ts, MtsNumChannelsT ch_ind);

    /**
     * @brief Declare the envelope groups with optimal size
     * @param num_env Number of envelopes per time series
     * @param l_min Minimum length of a query
     * @param l_max Maximum length of a query
     * @param lg_segmentation_strategy ILengthGroupSegmentationStrategy to use
     * @param ch_ind The channel index
     */
    vec<vec<Envelope>> get_envelope_groups(const uint series_len, const uint pos_per_env, const uint l_min,
                                           const uint l_max,
                                           const ILengthGroupSegmentationStrategy *lg_segmentation_strategies,
                                           MtsNumChannelsT ch_ind);

    /**
     * @brief Helper function to flip the values of envelope segments without data
     * @param envelope_groups Vector of envelope groups
     */
    void flip_env_infinities(vec<vec<Envelope>> &envelope_groups);

    friend class EnvelopeTest;
};

#endif  // INDEX_ENTRYGENERATOR_ENVELOPEENTRYGENERATOR_HPP
