#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESTATSSCORES_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESTATSSCORES_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScores.hpp"
#include "Util/Stats/IndexStats.hpp"
#include "Util/Types/Pointers.hpp"

class IndexStatsScoreFunc;

class EnvelopeStatsScores : public IEnvelopeScores {
   public:
    ~EnvelopeStatsScores();

    EnvelopeStatsScores(MtsNumChannelsT num_channels, uptr<IndexStatsScoreFunc> index_stats_score_func);

    bool update(const vec<Envelope> &mts_envelope) override;

    vec<Real> get_scores() override;

   private:
    vec<IndexStats> m_channel_stats;
    uptr<IndexStatsScoreFunc> m_index_stats_score_func;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASED_ENVELOPESTATSSCORES_HPP
