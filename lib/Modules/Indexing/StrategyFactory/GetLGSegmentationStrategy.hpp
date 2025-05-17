#ifndef MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP
#define MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP

#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"

uptr<ILengthGroupSegmentationStrategy> get_lg_segmentation_strategy(const IndexOptions &opts);

#endif  // MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP
