#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESCOREESTIMATOR_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESCOREESTIMATOR_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

class IEnvelopeScoreFunc;
class ScoreBasedChSSParams;

class EnvelopeScoreEstimator {
   public:
    EnvelopeScoreEstimator(uptr<IEnvelopeScoreFunc> envelope_scores);

    vec<Real> estimate_scores(const ScoreBasedChSSParams &sampling_params);

    vec<Real> get_scores() const;

   private:
    uptr<IEnvelopeScoreFunc> m_envelope_scores;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESCOREESTIMATOR_HPP
