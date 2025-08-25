#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"

#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/AdaptiveMultiLGSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/MultiLGSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"
#include "Modules/Indexing/StrategyFactory/GetChSegmentationStrategy.hpp"

uptr<ILengthGroupSegmentationStrategy> get_lg_segmentation_strategy(const GeneralIndexProperties &opts,
                                                                    const vec<Real> *channel_scores) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    if (!index_params) {
        throw std::runtime_error("Index options must contain PaaIndexParams for length group segmentation strategy.");
    }

    switch (index_params->m_segmentation_params.m_lg_strategy_type) {
        case SINGLE:
            return std::make_unique<SingleLGSegmentationStrategy>(get_ch_segmentation_strategy(
                opts, opts.m_l_min, opts.m_l_max, index_params->m_segmentation_params.m_num_segments, channel_scores));
        case MULTI:
            return std::make_unique<MultiLGSegmentationStrategy>(
                [&opts, channel_scores, index_params](uint lg_l_min, uint lg_l_max) {
                    return get_ch_segmentation_strategy(
                        opts, lg_l_min, lg_l_max, index_params->m_segmentation_params.m_num_segments, channel_scores);
                });
        case ADAPTIVE_MULTI:
            uint pos_per_env = 0;
            if (auto *env_params = dynamic_cast<const EnvelopeIndexParams *>(opts.m_index_params.get())) {
                pos_per_env = env_params->m_pos_per_env;
            }
            return std::make_unique<AdaptiveMultiLGSegmentationStrategy>(
                [&opts, channel_scores](uint lg_l_min, uint lg_l_max, SaxSegIndT num_segments) {
                    return get_ch_segmentation_strategy(opts, lg_l_min, lg_l_max, num_segments, channel_scores);
                },
                index_params->m_segmentation_params.m_num_segments, pos_per_env);
    }
    return nullptr;
}
