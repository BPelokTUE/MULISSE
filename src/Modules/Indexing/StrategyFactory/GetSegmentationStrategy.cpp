#include "Modules/Indexing/StrategyFactory/GetSegmentationStrategy.hpp"

#include "Index/Segmentation/SegmentationStrategy/AdaptiveSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"

sptr<ISegmentationStrategy> get_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                      SaxSegIndT num_segments) {
    auto index_params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());

    if (num_segments > l_max) {
        throw std::invalid_argument("Number of segments must be less equal than l_max");
    }

    switch (index_params->m_segmentation_params.m_strategy_type) {
        case UNIFORM:
            return std::make_unique<UniformSegmentationStrategy>(l_max, num_segments);
        case ADAPTIVE:
            uint pos_per_env = 0;
            if (auto *env_params = dynamic_cast<const EnvelopeIndexParams *>(opts.m_index_params.get())) {
                pos_per_env = env_params->m_pos_per_env;
            }
            return std::make_unique<AdaptiveSegmentationStrategy>(l_min, l_max, opts.m_series_len, num_segments,
                                                                  pos_per_env);
    }
    return nullptr;
}
