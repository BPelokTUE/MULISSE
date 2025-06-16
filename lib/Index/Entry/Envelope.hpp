#ifndef INDEX_ENTRY_ENVELOPE_HPP
#define INDEX_ENTRY_ENVELOPE_HPP

#include "Index/Entry/EntryData.hpp"
#include "Util/Constants/Math.hpp"

class ILengthGroupSegmentationStrategy;

/**
 * @brief Envelope of a multivariate time series
 *
 * An envelope is a set of lower and upper bounds that summarizes subsequences of a multivariate time series
 */
struct Envelope : IEntryData {
    /** @brief Lower bounds of the envelope */
    vec<Real> m_lower;
    /** @brief Upper bounds of the envelope */
    vec<Real> m_upper;

    Envelope() = default;

    /**
     * @brief Construct a new Envelope object
     * @param lower Lower bounds of the envelope
     * @param upper Upper bounds of the envelope
     */
    Envelope(vec<Real> lower, vec<Real> upper);

    /**
     * @brief Equality operator
     * @param other The other envelope to compare with
     */
    bool operator==(const Envelope &other) const;

    /**
     * @brief Merge the envelope with another, by selecting the minimum of the lower and maximum of the upper
     * in each segment
     * @param other The other envelope to merge with
     */
    inline void merge(const Envelope &other) {
        for (size_t i = 0; i < m_lower.size(); ++i) {
            m_lower[i] = std::min(m_lower[i], other.m_lower[i]);
            m_upper[i] = std::max(m_upper[i], other.m_upper[i]);
        }
    }

    inline size_t size() const override { return m_lower.size(); }

    void resize(size_t new_size) override;

    const vec<Real> &get_isax_input() const override;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_lower, m_upper);
    }
};

struct EnvelopeParams {
    /** @brief The minimum length of a subsequence */
    uint m_l_min;
    /** @brief The maximum length of a subsequence */
    uint m_l_max;
    /** @brief The (max) number of master series in each envelope */
    uint m_pos_per_env;
    /** @brief The segmentation strategies to use for each length group. */
    const ILengthGroupSegmentationStrategy *m_lg_segmentation_strategy;
};

#endif  // INDEX_ENTRY_ENVELOPE_HPP
