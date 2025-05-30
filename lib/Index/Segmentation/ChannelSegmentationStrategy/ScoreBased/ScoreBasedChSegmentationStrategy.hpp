#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/MultiChSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

class IScoreToSegmentationStrategy;

class ScoreBasedChSegmentationStrategy : public MultiChSegmentationStrategy {
   public:
    ~ScoreBasedChSegmentationStrategy();

    ScoreBasedChSegmentationStrategy();

    /**
     * @brief Constructor for ScoreBasedChSegmentationStrategy.
     * @param channel_scores Vector of channel scores
     * @param score_to_segmentation_strategy Function for converting scores to segmentation strategies
     */
    ScoreBasedChSegmentationStrategy(const vec<Real> &channel_scores,
                                     uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy);
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
