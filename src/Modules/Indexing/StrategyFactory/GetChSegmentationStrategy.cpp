#include "Modules/Indexing/StrategyFactory/GetChSegmentationStrategy.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/MultiChSegmentationStrategy.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeStatsScores.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSegmentationStrategy.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToProportionalNumSegments.hpp"
#include "Modules/Indexing/StrategyFactory/GetSegmentationStrategy.hpp"
#include "Util/Stats/IndexStatsExtractor/EnvelopeShapeExtractor.hpp"
#include "Util/Stats/ScoreFunc/IndexStatsScoreFunc.hpp"
#include "Util/Stats/ScoreFunc/WeightedScoreFunc.hpp"

using CHSS = ChannelSegmentationStrategyType;

sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                                SaxSegIndT num_segments) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    switch (index_params->m_segmentation_params.m_ch_strategy_type) {
        case CHSS::SINGLE:
            return std::make_unique<SingleChSegmentationStrategy>(
                get_segmentation_strategy(opts, l_min, l_max, num_segments));
        case CHSS::MULTI:
            return std::make_unique<MultiChSegmentationStrategy>(
                [&opts, l_min, l_max](SaxSegIndT num_prop_segments) {
                    return get_segmentation_strategy(opts, l_min, l_max, num_prop_segments);
                },
                num_segments, index_params->m_segmentation_params.m_ch_num_seg_props_file);
        case CHSS::ENV_STATS_BASED: {
            auto sampling_params = index_params->m_segmentation_params.m_ch_sampling_params;
            if (!sampling_params) throw std::runtime_error("SamplingChSegmentationStrategy requires sampling params");
            auto score_based_params =
                dynamic_cast<const EnvStatsChSSParams *>(index_params->m_segmentation_params.m_ch_score_based_params);
            if (!score_based_params) {
                throw std::runtime_error("ScoreBasedChSegmentationStrategy requires envelope stats params");
            }

            auto index_stats_score_func = std::make_unique<IndexStatsScoreFunc>(
                std::make_unique<EnvelopeShapeExtractor>(),
                std::make_unique<WeightedScoreFunc>(score_based_params->m_weights_file));
            auto env_stats_scores = std::make_unique<EnvelopeStatsScores>(std::move(index_stats_score_func));

            auto score_to_strategy = std::make_unique<ScoreToProportionalNumSegments>(
                num_segments, opts.m_num_channels,
                [&opts, l_min, l_max](SaxSegIndT num_prop_segments) {
                    return get_segmentation_strategy(opts, l_min, l_max, num_prop_segments);
                },
                score_based_params->m_prop_exp);

            return std::make_unique<ScoreBasedChSegmentationStrategy>(std::move(env_stats_scores),
                                                                      std::move(score_to_strategy), *sampling_params);
        }
        case CHSS::ENV_WIDTH_BASED:
            throw std::runtime_error("WidthBasedChSegmentationStrategy is not implemented yet");
    }
    return nullptr;
}
