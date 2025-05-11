#ifndef MODULES_QUERYGEN_HPP
#define MODULES_QUERYGEN_HPP

#include "Search/QuerySetOptions.hpp"

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
 * @param opts Options for generating the queries
 */
int create_queries(QuerySetOptions opts);

#endif  // MODULES_QUERYGEN_HPP
