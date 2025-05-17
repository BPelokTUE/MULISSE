#ifndef MODULES_INDEXING_GETCHSEGMENTATIONSTRATEGY_HPP
#define MODULES_INDEXING_GETCHSEGMENTATIONSTRATEGY_HPP

#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                                SaxSegIndT num_segments);

#endif  // MODULES_INDEXING_GETCHSEGMENTATIONSTRATEGY_HPP
