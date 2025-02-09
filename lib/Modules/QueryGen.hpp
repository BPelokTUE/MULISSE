#ifndef QUERY_GEN_HPP
#define QUERY_GEN_HPP

#include "Util/typedefs.hpp"

/**
 * @brief Create queries from dataset by extracting subsequences and adding noise
 *
 * This function reads a dataset from a binary file and creates queries by extracting subsequences
 * of specified lengths at random points, from random series, containing a random non-empty subset
 * of channels and adding noise to them. The queries are saved to a text file.
 *
 * @param noise Noise to add to the queries
 * @param num_series Number of series in the dataset
 * @param num_channels Number of channels in each series
 * @param num_queries Number of queries to generate per length in `lengths`
 * @param lengths Lengths of the queries. For each length `num_queries` queries will be generated.
 * @param seed Seed for the random number generator
 * @return 0 on success, 1 if dataset does not exist
 */
int create_queries(float noise, uint num_series, uint num_channels, uint num_queries, vec<uint> lengths, int seed);

#endif  // QUERY_GEN_HPP
