#ifndef UTIL_TYPES_MTSQUERY_HPP
#define UTIL_TYPES_MTSQUERY_HPP

#include "Util/Types/MultivariateTimeSeries.hpp"

class MtsQuery : public MultivariateTimeSeries {
   private:
    vec<bool> m_used_channels;
    uint m_query_len;

   public:
    /** @brief Construct MtsQuery from data */
    MtsQuery(const vec<vec<Real>> &&data);

    /**
     * @brief Check whether a channel is used in the query
     * @param channel_ind Index of the channel to check
     * @return Whether the channel is used (is non-empty)
     * @throws std::out_of_range if the channel index is out of range
     */
    bool is_channel_used(MtsNumChannelsT channel_ind);

    /**
     * @brief Get the length of the query
     * @return Length of the query
     */
    uint get_query_len() const;
};

#endif  // UTIL_TYPES_MTSQUERY_HPP
