#ifndef QUERY_SET_OPTIONS
#define QUERY_SET_OPTIONS

#include "Util/typedefs.hpp"

struct QuerySetOptions {
    /** @brief Noise to add to the queries */
    Real m_noise;
    /** @brief Number of queries to generate per length in `lengths` */
    uint m_num_queries;
    /** @brief Exact lengths of the queries. For each length `num_queries` queries will be generated. Overriden by
     * `l_min` and `l_max`. */
    vec<uint> m_exact_lengths;
    /** @brief Minimum length of the queries to generate. If passed `l_max` is also required. Overrides `exact_lengths`.
     */
    uint m_l_min;
    /** @brief Maximum length of the queries to generate. If passed `l_min` is also required. Overrides `exact_lengths`.
     */
    uint m_l_max;
    /** @brief The number of channels to use for each query. If 0, the number is random for each query. */
    MtsNumChannelsT m_used_channels;
    /** @brief The mask describing which channels to use in the queries. Overrides `used_channels` if provided. */
    vec<bool> m_channel_mask;
    /** @brief Seed for the random number generator */
    uint m_seed;
};

#endif  // QUERY_SET_OPTIONS
