#ifndef INDEX_SEGMENTATION_SCORETOSEGMENTATIONSTRATEGY_SCORETOSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_SCORETOSEGMENTATIONSTRATEGY_SCORETOSEGMENTATIONSTRATEGY_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

class ISegmentationStrategy;

/** @brief Interface for factories that generate segmentation strategies based on scores */
class IScoreToSegmentationStrategy {
   public:
    virtual ~IScoreToSegmentationStrategy() = default;

    /**
     * @brief Get a segmentation strategies based on the specified scores.
     * @param scores The scores to use.
     * @return The segmentation strategies.
     */
    virtual vec<sptr<ISegmentationStrategy>> get_segmentation_strategies(const vec<Real> &scores) const = 0;
};

#endif  // INDEX_SEGMENTATION_SCORETOSEGMENTATIONSTRATEGY_SCORETOSEGMENTATIONSTRATEGY_HPP
