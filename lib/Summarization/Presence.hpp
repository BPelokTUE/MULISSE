#ifndef PRESENCE_HPP
#define PRESENCE_HPP

#include "Util/typedefs.hpp"

class PresenceArray {
   public:
    /**
     * @brief Calculate the presence of each point and the total presence sum.
     * @param l_min Minimum length of queries
     * @param l_max Maximum length of queries
     * @param series_len Length of the whole time series
     * @param pos_per_env Number of positions per envelope, defaults to 0 indicating no enveloping
     * @return A pair containing the presence of each point and the total presence sum.
     */
    PresenceArray(uint l_min, uint l_max, uint series_len, uint pos_per_env);

    /**
     * @brief Get the presence of each point.
     * @return A vector containing the presence of each point.
     */
    const vec<size_t> &get_presences() const;

    /**
     * @brief Get the total presence sum.
     * @return The total presence sum.
     */
    size_t get_presence_sum() const;

   private:
    vec<size_t> m_presences;
    size_t m_presence_sum;
};

#endif  // PRESENCE_HPP
