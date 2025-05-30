#include "Modules/Indexing/GetChannelScores.hpp"

#include "Enums/EnvelopeScoresTypes.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScoreEstimator.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScoreFunc.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeStatsScoreFunc.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeWidthScoreFunc.hpp"
#include "Util/Stats/IndexStatsExtractor/EnvelopeShapeExtractor.hpp"
#include "Util/Stats/ScoreFunc/IndexStatsScoreFunc.hpp"
#include "Util/Stats/ScoreFunc/WeightedScoreFunc.hpp"

vec<Real> get_channel_scores(const IndexOptions &opts) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    if (!index_params) {
        throw std::runtime_error("Index options must contain PaaIndexParams for channel scores.");
    }

    if (auto score_based_chss_params = index_params->m_segmentation_params.m_score_based_chss_params) {
        uptr<IEnvelopeScoreFunc> envelope_score_func;
        switch (score_based_chss_params->m_env_scores_type) {
            case STATS: {
                auto index_stats_score_func = std::make_unique<IndexStatsScoreFunc>(
                    std::make_unique<EnvelopeShapeExtractor>(),
                    std::make_unique<WeightedScoreFunc>(score_based_chss_params->m_weights_file));
                envelope_score_func =
                    std::make_unique<EnvelopeStatsScoreFunc>(opts.m_num_channels, std::move(index_stats_score_func));
            }
            case WIDTH: {
                envelope_score_func = std::make_unique<EnvelopeWidthScoreFunc>(
                    opts.m_num_channels, score_based_chss_params->m_min_width_update);
            }
        }
        return EnvelopeScoreEstimator(std::move(envelope_score_func)).get_scores();
    }
    return {};
}
