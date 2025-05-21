#ifndef INDEX_SEGMENTATION_SCORETOSEGMENTATIONSTRATEGY_SCORETOPROPORTIONALNUMSEGMENTS_HPP
#define INDEX_SEGMENTATION_SCORETOSEGMENTATIONSTRATEGY_SCORETOPROPORTIONALNUMSEGMENTS_HPP

#include <functional>

#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToSegmentationStrategy.hpp"

using NumSegToSegmentationStrategy = std::function<sptr<ISegmentationStrategy>(SaxSegIndT)>;

class ScoreToProportionalNumSegments : public IScoreToSegmentationStrategy {
   public:
    /**
     * @brief Construct a new ScoreToProportionalNumSegments object
     * @param num_segments_total The total number of segments to use
     * @param strategy_factory The function to use for creating segmentation strategies
     */
    ScoreToProportionalNumSegments(uint num_segments_total, const NumSegToSegmentationStrategy &strategy_factory);

    /**
     * @brief Construct a new ScoreToProportionalNumSegments object
     * @param num_segments_avg The average number of segments to use per channel
     * @param num_channels The number of channels
     * @param strategy_factory The function to use for creating segmentation strategies
     */
    ScoreToProportionalNumSegments(SaxSegIndT num_channels_avg, MtsNumChannelsT num_channels,
                                   const NumSegToSegmentationStrategy &strategy_factory);

    vec<sptr<ISegmentationStrategy>> get_segmentation_strategy(const vec<Real> &scores) const override;

   private:
    uint m_num_segments_total;
    NumSegToSegmentationStrategy m_strategy_factory;
};

#endif  // INDEX_SEGMENTATION_SCORETOSEGMENTATIONSTRATEGY_SCORETOPROPORTIONALNUMSEGMENTS_HPP
