#ifndef MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP
#define MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"
#include "Util/Types/Vec.hpp"

class ILengthGroupSegmentationStrategy;
class GeneralIndexProperties;

/**
 * @brief Get the length group segmentation strategy based on the index options.
 * @param opts The index options containing the segmentation parameters.
 * @param channel_scores The scores for each channel, used by ScoreBasedChSegmentationStrategy.
 * @return The length group segmentation strategy.
 */
uptr<ILengthGroupSegmentationStrategy> get_lg_segmentation_strategy(const GeneralIndexProperties &opts,
                                                                    const vec<Real> *channel_scores = nullptr);

#endif  // MODULES_INDEXING_GETLGSEGMENTATIONSTRATEGY_HPP
