#ifndef MODULES_INDEXING_INDEXFACTORYPARAMS_HPP
#define MODULES_INDEXING_INDEXFACTORYPARAMS_HPP

#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

struct IndexFactoryParams {
    bool m_discretize_flat_index = false;
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
    const IndexOptions &m_opts;
};

#endif  // MODULES_INDEXING_INDEXFACTORYPARAMS_HPP
