#ifndef MODULES_QUERYSTATS_HPP
#define MODULES_QUERYSTATS_HPP

/**
 * @brief Calculate statistics of a query set
 *
 * This function computes statistics for a query set, such as the minimum, maximum, mean and standard deviation of each
 * query to subsequences in the dataset (these can be used to calculate relative contrast for example).
 *
 * @param normalized Whether to normalize the queries
 */
int calculate_query_stats(bool normalized);

#endif  // MODULES_QUERYSTATS_HPP
