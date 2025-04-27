#ifndef SEGMENTATION_STRATEGY_HPP
#define SEGMENTATION_STRATEGY_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enum for IiSaxSplitStrategy implementations */
enum SegmentationStrategyType { UNIFORM, ADAPTIVE };

DEFINE_ENUM_CONSTS(SegmentationStrategyType, SEGMENTATION_STRATEGY_TYPE, false,
                   (umap<str, SegmentationStrategyType>{{"equi_width", UNIFORM}, {"equi_depth", ADAPTIVE}}));

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
        ar(m_segment_len);
    }
};

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
     * @param num_segments Number of segments
     */
    AdaptiveSegmentationStrategy(uint l_min, uint l_max, uint series_len, SaxSegIndT num_segments);

    AdaptiveSegmentationStrategy() = default;

    SaxSegIndT get_num_segments(uint subs_len) const override;

    uint get_segment_len(SaxSegIndT segment_ind) const override;

    SegmentationStrategyType get_type() const override;

   private:
    uint m_l_max;
    vec<uint> m_segment_lens, m_segment_ends;

    /**
     * @brief Calculate the presence of each point and the total presence sum.
     * @param l_min Minimum length of queries
     * @param l_max Maximum length of queries
     * @param series_len Length of the whole time series
     * @return A pair containing the presence of each point and the total presence sum.
     */
    std::pair<vec<size_t>, size_t> calculate_presences(uint l_min, uint l_max, uint series_len) const;

    friend class AdaptiveSegmentationStrategyTest;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_segment_lens, m_segment_ends);
    }
};

#endif  // SEGMENTATION_STRATEGY_HPP
