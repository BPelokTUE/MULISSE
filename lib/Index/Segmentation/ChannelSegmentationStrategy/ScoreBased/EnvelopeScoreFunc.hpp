#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESCOREFUNC_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESCOREFUNC_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

class Envelope;

class IEnvelopeScoreFunc {
   public:
    virtual ~IEnvelopeScoreFunc() = default;

    /**
     * @brief Update the envelope scores based on the given (MTS) envelope.
     * @param mts_envelope The envelope to update scores with.
     * @return `true` if the update was sufficient, `false` otherwise.
     */
    virtual bool update(const vec<Envelope> &mts_envelope) = 0;

    /**
     * @brief Get the score for the envelopes.
     * @return A vector of scores for each envelope.
     */
    virtual vec<Real> get_scores() = 0;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESCOREFUNC_HPP
