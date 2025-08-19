#ifndef UTIL_TYPES_MTSQUERY_HPP
#define UTIL_TYPES_MTSQUERY_HPP

#include "Util/Types/MultivariateTimeSeries.hpp"

class MtsQuery : public MultivariateTimeSeries {
   private:
    /** @brief Whether the query is normalized */
    bool m_normalized;
    /** @brief Vector of booleans indicating whether each channel is used in the query */
    vec<bool> m_used_channels;
    /** @brief Length of the query */
    uint m_query_len;

   public:
    /**
     * @brief Construct MtsQuery from data
     * @param data Multivariate time series data
     * @param normalized Whether the query is normalized
     */
    MtsQuery(const vec<vec<Real>> &&data, bool normalized = true);

    /**
     * @brief Get whether the query is normalized
     * @return Whether the query is normalized
     */
    bool is_normalized() const;

    /**
     * @brief Check whether a channel is used in the query
     * @param channel_ind Index of the channel to check
     * @return Whether the channel is used (is non-empty)
     * @throws std::out_of_range if the channel index is out of range
     */
    bool is_channel_used(MtsNumChannelsT channel_ind) const;

    /**
     * @brief Get the used channels mask
     * @return Vector of booleans indicating whether each channel is used
     */
    const vec<bool> &get_used_channels() const;

    /**
     * @brief Get the length of the query
     * @return Length of the query
     */
    uint get_query_len() const;
};

#endif  // UTIL_TYPES_MTSQUERY_HPP
