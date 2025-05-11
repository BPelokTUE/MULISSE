#ifndef PAA_HPP
#define PAA_HPP

#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Piecewise Aggregate Approximation (PAA) of a time series
 *
 * This function computes the Piecewise Aggregate Approximation (PAA) of a time series
 *
 * @param ts The time series to approximate
 * @param segmentation_strategy The segmentation strategy to use
 * @return The PAA of the time series
 */
vec<Real> paa(const vec<Real> &ts, const ISegmentationStrategy *segmentation_strategy);

struct PaaParams {
    /** @brief The minimum length of a subsequence */
    uint m_l_min;
    /** @brief The maximum length of a subsequence */
    uint m_l_max;
    /** @brief The segmentation strategies to use for each length group. If it only contains a single strategy, that is
     * used across all length groups. */
    const ILengthGroupSegmentationStrategy *m_lg_segmentation_strategy;
};

struct Paa : IEntryData {
    vec<Real> m_paa_values;

    /**
     * @brief Construct a new Paa object
     * @param paa_values The PAA values of the time series
     */
    Paa(const vec<Real> &paa_values);

    Paa() = default;

    /**
     * @brief Equality operator
     * @param other The other PAA to compare with
     */
    bool operator==(const Paa &other) const;

    /**
     * @brief Merge the Paa with another, by selecting the minimum of each segment
     * @param other The other PAA to merge with
     */
    inline void merge(const Paa &other) {
        for (size_t i = 0; i < m_paa_values.size(); ++i) {
            m_paa_values[i] = std::min(m_paa_values[i], other.m_paa_values[i]);
        }
    }

    size_t size() const override;

    void resize(size_t new_size) override;

    const vec<Real> &get_isax_input() const override;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_paa_values);
    }
};

#endif  // PAA_HPP
