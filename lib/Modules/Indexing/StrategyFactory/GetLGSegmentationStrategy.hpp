#ifndef MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP
#define MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

class ILengthGroupSegmentationStrategy;
class IndexOptions;

/**
 * @brief Get the length group segmentation strategy based on the index options.
 * @param opts The index options containing the segmentation parameters.
 * @param channel_scores The scores for each channel, used by ScoreBasedChSegmentationStrategy.
 * @return The length group segmentation strategy.
 */
uptr<ILengthGroupSegmentationStrategy> get_lg_segmentation_strategy(const IndexOptions &opts,
                                                                    const vec<Real> &channel_scores);

#endif  // MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP
