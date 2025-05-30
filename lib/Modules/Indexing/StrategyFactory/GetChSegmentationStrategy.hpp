#ifndef MODULES_INDEXING_GETCHSEGMENTATIONSTRATEGY_HPP
#define MODULES_INDEXING_GETCHSEGMENTATIONSTRATEGY_HPP

#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

/**
 * @brief Get the channel segmentation strategy for the given length group settings
 * @param opts The index options containing the segmentation parameters
 * @param l_min The minimum length of the queries in the length group
 * @param l_max The maximum length of the queries in the length group
 * @param num_segments The number of segments to use per channel on average
 */
sptr<IChannelSegmentationStrategy> get_ch_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                                SaxSegIndT num_segments);

#endif  // MODULES_INDEXING_GETCHSEGMENTATIONSTRATEGY_HPP
