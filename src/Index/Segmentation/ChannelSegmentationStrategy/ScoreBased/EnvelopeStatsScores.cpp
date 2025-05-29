#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeStatsScores.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Util/Stats/ScoreFunc/IndexStatsScoreFunc.hpp"

EnvelopeStatsScores::~EnvelopeStatsScores() = default;

EnvelopeStatsScores::EnvelopeStatsScores(MtsNumChannelsT num_channels, uptr<IndexStatsScoreFunc> index_stats_score_func)
    : m_channel_stats(num_channels), m_index_stats_score_func(std::move(index_stats_score_func)) {}

bool EnvelopeStatsScores::update(const vec<Envelope> &mts_envelope) {
    for (MtsNumChannelsT c = 0; c < mts_envelope.size(); ++c) {
        auto &envelope = mts_envelope[c];
        for (SaxSegIndT s = 0; s < envelope.size(); ++s) {
            Real lower = envelope.m_lower[s], upper = envelope.m_upper[s];
            m_channel_stats[c].update_seg_stats(lower, upper, c, s);
        }
    }
    return true;
}

vec<Real> EnvelopeStatsScores::get_scores() {
    vec<Real> scores;
    scores.reserve(m_channel_stats.size());
    for (auto &ch_stat : m_channel_stats) {
        ch_stat.calculate();
        scores.push_back(m_index_stats_score_func->calculate_score(ch_stat));
    }
    return scores;
}
