#ifndef INDEX_SEGMENTATION_SEGMENTATIONSTRATEGY_ADAPTIVESEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_SEGMENTATIONSTRATEGY_ADAPTIVESEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"

/**
 * @brief Adaptive (equi-depth) segmentation strategy
 *
 * This strategy divides the time series (subsequence) such that each segment has roughly the same presence across all
 * relevant subsequences. The presence of a segment is defined as the sum of presences of the points within the segment.
 * The presence of a point is the number of relevant (l between l_min and l_max) subsequences that contain the point.
 */
class AdaptiveSegmentationStrategy : public ISegmentationStrategy {
   public:
    /**
     * @brief Construct a new AdaptiveSegmentationStrategy with the given segment length.
     * @param l_min Minimum length of queries
     * @param l_max Maximum length of queries
     * @param series_len Length of the whole time series
     * @param avg_num_segments Number of segments
     * @param pos_per_env Number of positions per envelope, defaults to 0 indicating no enveloping
     */
    AdaptiveSegmentationStrategy(uint l_min, uint l_max, uint series_len, SaxSegIndT num_segments,
                                 uint pos_per_env = 0);

    AdaptiveSegmentationStrategy() = default;

    SaxSegIndT get_num_segments(uint subs_len) const override;

    uint get_segment_len(SaxSegIndT segment_ind) const override;

    SegmentationStrategyType get_type() const override;

   private:
    uint m_l_max;
    vec<uint> m_segment_lens, m_segment_ends;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_l_max, m_segment_lens, m_segment_ends);
    }
};

#endif  // INDEX_SEGMENTATION_SEGMENTATIONSTRATEGY_ADAPTIVESEGMENTATIONSTRATEGY_HPP
