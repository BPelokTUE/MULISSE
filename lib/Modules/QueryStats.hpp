#ifndef QUERY_STATS_HPP
#define QUERY_STATS_HPP

/**
 * @brief Calculate statistics of a query set
 *
 * This function computes statistics for a query set, such as the minimum, maximum, mean and standard deviation of each
 * query to subsequences in the dataset (these can be used to calculate relative contrast for example).
 *
 * @param normalized Whether to normalize the queries
 * @param noise The standard deviation of the Gaussian noise used for generating the random walk query
 */
int calculate_query_stats(bool normalized, float noise);

#endif  // QUERY_STATS_HPP
