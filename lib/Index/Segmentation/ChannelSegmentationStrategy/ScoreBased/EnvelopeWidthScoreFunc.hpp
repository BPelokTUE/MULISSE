#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPEWIDTHSCOREFUNC_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPEWIDTHSCOREFUNC_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScoreFunc.hpp"
#include "Util/Types/Numbers.hpp"

class EnvelopeWidthScoreFunc : public IEnvelopeScoreFunc {
   public:
    EnvelopeWidthScoreFunc(MtsNumChannelsT num_channels, Real min_width_update = 0.0);

    bool update(const vec<Envelope> &mts_envelope) override;

    vec<Real> get_scores() override;

   private:
    Real m_min_width_update;
    vec<Real> m_range_sums;
    uint m_sample_count = 0;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPEWIDTHSCOREFUNC_HPP
