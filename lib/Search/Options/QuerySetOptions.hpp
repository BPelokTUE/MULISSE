#ifndef QUERY_SET_OPTIONS
#define QUERY_SET_OPTIONS

#include "Util/typedefs.hpp"

struct QuerySetOptions {
    /** @brief Noise to add to the queries */
    float noise;
    /** @brief Number of queries to generate per length in `lengths` */
    uint num_queries;
    /** @brief Exact lengths of the queries. For each length `num_queries` queries will be generated. Overriden by
     * `l_min` and `l_max`. */
    vec<uint> exact_lengths;
    /** @brief Minimum length of the queries to generate. If passed `l_max` is also required. Overrides `exact_lengths`.
     */
    uint l_min;
    /** @brief Maximum length of the queries to generate. If passed `l_min` is also required. Overrides `exact_lengths`.
     */
    uint l_max;
    /** @brief The number of channels to use for each query. If 0, the number is random for each query. */
    MtsNumChannelsT used_channels;
    /** @brief The mask describing which channels to use in the queries. Overrides `used_channels` if provided. */
    vec<bool> channel_mask;
    /** @brief Seed for the random number generator */
    int seed;
};

#endif  // QUERY_SET_OPTIONS
