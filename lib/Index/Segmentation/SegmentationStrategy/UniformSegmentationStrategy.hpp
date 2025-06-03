#ifndef INDEX_SEGMENTATION_SEGMENTATIONSTRATEGY_UNIFORMSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_SEGMENTATIONSTRATEGY_UNIFORMSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"

/**
 * @brief Uniform (equi-width) segmentation strategy
 *
 * This strategy divides the time series (subsequence) into equal length segments.
 */
class UniformSegmentationStrategy : public ISegmentationStrategy {
   public:
    /**
     * @brief Construct a new UniformSegmentationStrategy with the given segment length.
     * @param l_max Maximum length of queries
     * @param num_segments Number of segments
     */
    UniformSegmentationStrategy(uint l_max, SaxSegIndT num_segments);

    UniformSegmentationStrategy() = default;

    SaxSegIndT get_num_segments(uint subs_len) const override;

    uint get_segment_len(SaxSegIndT segment_ind) const override;

    SegmentationStrategyType get_type() const override;

   private:
    uint m_segment_len, m_l_max;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_segment_len, m_l_max);
    }
};

#endif  // INDEX_SEGMENTATION_SEGMENTATIONSTRATEGY_UNIFORMSEGMENTATIONSTRATEGY_HPP
