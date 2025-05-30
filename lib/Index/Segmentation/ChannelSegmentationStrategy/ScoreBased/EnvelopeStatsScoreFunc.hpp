#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESTATSSCOREFUNC_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESTATSSCOREFUNC_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScoreFunc.hpp"
#include "Util/Types/Pointers.hpp"

class IndexStatsScoreFunc;
class IndexStats;

class EnvelopeStatsScoreFunc : public IEnvelopeScoreFunc {
   public:
    ~EnvelopeStatsScoreFunc();

    EnvelopeStatsScoreFunc(MtsNumChannelsT num_channels, uptr<IndexStatsScoreFunc> index_stats_score_func);

    bool update(const vec<Envelope> &mts_envelope) override;

    vec<Real> get_scores() override;

   private:
    vec<IndexStats> m_channel_stats;
    uptr<IndexStatsScoreFunc> m_index_stats_score_func;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESTATSSCOREFUNC_HPP
