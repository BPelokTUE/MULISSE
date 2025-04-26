#ifndef SEGMENTATION_STRATEGY_HPP
#define SEGMENTATION_STRATEGY_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enum for IiSaxSplitStrategy implementations */
enum SegmentationStrategyType { UNIFORM };

DEFINE_ENUM_CONSTS_NO_EXTRA(SegmentationStrategyType, SEGMENTATION_STRATEGY_TYPE, false);

/** @brief Interface for segmentation strategies. */
class ISegmentationStrategy {
   public:
    virtual ~ISegmentationStrategy() = default;

    /**
     * @brief Get the number of segments for a given time series (subsequence) size.
     * @param ts_size Size of the time series (subsequence).
     * @return Number of segments.
     */
    virtual SaxSegIndT get_num_segments(uint ts_size) const = 0;

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

/**
 * @brief Uniform segmentation strategy
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

    SaxSegIndT get_num_segments(uint ts_size) const override;

    uint get_segment_len(SaxSegIndT segment_ind) const override;

    SegmentationStrategyType get_type() const override;

   private:
    uint m_segment_len;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_segment_len);
    }
};

#endif  // SEGMENTATION_STRATEGY_HPP
