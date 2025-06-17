#include "Modules/Indexing/StrategyFactory/GetChSegmentationStrategy.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/MultiChSegmentationStrategy.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSegmentationStrategy.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreToProportionalNumSegments.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Modules/Indexing/StrategyFactory/GetSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Containers.hpp"

using CHSS = ChannelSegmentationStrategyType;

sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                                SaxSegIndT num_segments,
                                                                const vec<Real> *channel_scores) {
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
        case CHSS::SCORE_BASED: {
            if (!channel_scores) {
                throw std::runtime_error("Channel scores must be provided for ScoreBasedChSegmentationStrategy.");
            }
            auto score_to_strategy = std::make_unique<ScoreToProportionalNumSegments>(
                num_segments, opts.m_num_channels,
                [&opts, l_min, l_max](SaxSegIndT num_prop_segments) {
                    return get_segmentation_strategy(opts, l_min, l_max, num_prop_segments);
                },
                index_params->m_segmentation_params.m_score_based_chss_params->m_score_exp);

            return std::make_unique<ScoreBasedChSegmentationStrategy>(*channel_scores, std::move(score_to_strategy));
        }
    }
    return nullptr;
}
