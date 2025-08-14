#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_SCOREBASEDCHSSPARAMS_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_SCOREBASEDCHSSPARAMS_HPP

#include "Enums/EnvelopeScoresTypes.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

struct ScoreBasedChSSParams {
    /** @brief Whether to use normalized envelopes (subsequences) during sampling */
    bool m_normalized = true;
    /** @brief The segment length to use for the envelopes during sampling */
    uint m_segment_len = 1;
    /** @brief The sample size to use for sampling */
    uint m_sample_size = 0;
    /** @brief The exponent to use in ScoreToProportionalNumSegments */
    Real m_score_exp = 1.0;
    /** @brief The minimum change in the envelope width for a sufficient update */
    Real m_min_width_update = 0.0;
    /** @brief File containing the weights to use for WeightedScoreFunc */
    str m_weights_file = "";
    /** @brief Type of IEnvelopeScores implementation to use */
    EnvelopeScoresType m_env_scores_type;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_SCOREBASEDCHSSPARAMS_HPP
