#include "Modules/Indexing/StrategyFactory/GetChSegmentationStrategy.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Modules/Indexing/StrategyFactory/GetSegmentationStrategy.hpp"

using CHSS = ChannelSegmentationStrategyType;

sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                                SaxSegIndT num_segments) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    switch (index_params->m_segmentation_params.m_ch_strategy_type) {
        case CHSS::SINGLE:
            return std::make_unique<SingleChSegmentationStrategy>(
                get_segmentation_strategy(opts, l_min, l_max, num_segments));
    }
    return nullptr;
}
