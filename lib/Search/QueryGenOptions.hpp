#ifndef SEARCH_QUERYSETGENOPTIONS_HPP
#define SEARCH_QUERYSETGENOPTIONS_HPP

#include "Util/Types/LengthRange.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

/** @brief Options for generating queries */
struct QuerySetGenOptions {
    /** @brief The number of channels to use for each query. If 0, the number is random for each query. */
    MtsNumChannelsT m_used_channels;
    /** @brief Seed for the random number generator */
    uint m_seed;
    /** @brief Noise to add to the queries */
    Real m_noise;
    /** @brief Exact lengths of the queries. For each length `num_queries` queries will be generated. Overriden by
     * `l_min` and `l_max`. */
    vec<uint> m_exact_lengths;
    /** @brief The mask describing which channels to use in the queries. Overrides `used_channels` if provided. */
    vec<bool> m_channel_mask;
};

#endif  // SEARCH_QUERYSETGENOPTIONS_HPP
