#ifndef QUERY_GEN_HPP
#define QUERY_GEN_HPP

#include "Util/typedefs.hpp"

/**
 * @brief Create queries from dataset by extracting subsequences and adding noise
 *
 * This function reads a dataset from a binary file and creates queries by extracting subsequences
 * of specified lengths at random points, from random series, containing a random non-empty subset
 * of channels and adding noise to them. The queries are saved to a text file. To specify the length of the queries,
 * either pass a list of exact lengths, in which case `num_queries` queries will be generated for all of them, or a
 * minimum and maximum length, resulting in `num_queries` queries being generated with uniformly distributed length
 * between the provided limits.
 *
 * @param noise Noise to add to the queries
 * @param num_queries Number of queries to generate per length in `lengths`
 * @param exact_lengths Exact lengths of the queries. For each length `num_queries` queries will be generated. Overriden
 *        by `l_min` and `l_max`.
 * @param l_min Minimum length of the queries to generate. If passed `l_max` is also required. Overrides
 * `exact_lengths`.
 * @param l_max Maximum length of the queries to generate. If passed `l_min` is also required. Overrides
 * `exact_lengths`.
 * @param used_channels The number of channels to use for each query. If 0, the number is random for each query.
 * @param channel_mask The mask describing which channels to use in the queries. Overrides `used_channels` if provided.
 * @param seed Seed for the random number generator
 * @return 0 on success, 1 if dataset does not exist
 */
int create_queries(float noise, uint num_queries, vec<uint> lengths, uint l_min, uint l_max,
                   MtsNumChannelsT used_channels, vec<bool> channel_mask, int seed);

#endif  // QUERY_GEN_HPP
