#ifndef INDEX_SEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATIONSTRATEGY_HPP

#include <cereal/access.hpp>

#include "Enums/SegmentationStrategyType.hpp"
#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Interface for segmentation strategies. */
class ISegmentationStrategy {
   public:
    virtual ~ISegmentationStrategy() = default;

    /**
     * @brief Get the number of segments for a given time series (subsequence) size.
     * @param subs_len Size of the time series subsequence.
     * @return Number of segments.
     */
    virtual SaxSegIndT get_num_segments(uint subs_len) const = 0;

    /**
     * @brief Get the length of a given segment.
     * @param segment_ind The segment index.
     * @return The length of the segment.
     */
    virtual uint get_segment_len(SaxSegIndT segment_ind) const = 0;

    /**
     * @brief Get the type of the segmentation strategy.
     * @return The type of the segmentation strategy.
     */
    virtual SegmentationStrategyType get_type() const = 0;
};

#endif  // INDEX_SEGMENTATIONSTRATEGY_HPP
