#ifndef MODULES_INDEXING_GETSEGMENTATIONSTRATEGY_HPP
#define MODULES_INDEXING_GETSEGMENTATIONSTRATEGY_HPP

#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/Types/Pointers.hpp"

sptr<ISegmentationStrategy> get_segmentation_strategy(const IndexOptions &opts, uint l_min, uint l_max,
                                                      SaxSegIndT num_segments);

#endif  // MODULES_INDEXING_GETSEGMENTATIONSTRATEGY_HPP
