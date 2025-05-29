#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPEWIDTHSCORES_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPEWIDTHSCORES_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScores.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/Types/Numbers.hpp"

class EnvelopeWidthScores : public IEnvelopeScores {
   public:
    EnvelopeWidthScores(Real min_width_update = 0.0);

    bool update(const vec<Envelope> &mts_envelope) override;

    vec<Real> get_scores() override;

   private:
    Real m_min_width_update, m_range_min = INF, m_range_max = 0.0;
    vec<Real> m_range_sums;
    uint m_sample_count = 0;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPEWIDTHSCORES_HPP
