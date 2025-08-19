#ifndef MODULES_QUERYSTATS_HPP
#define MODULES_QUERYSTATS_HPP

class MtsQuerySet;
class QueryStatsLogger;

/**
 * @brief Calculate statistics of a query set
 *
 * This function computes statistics for a query set, such as the minimum, maximum, mean and standard deviation of each
 * query to subsequences in the dataset (these can be used to calculate relative contrast for example).
 *
 * @param query_set The MtsQuerySet to calculate statistics for
 * @param normalized Whether to normalize the queries
 * @param logger The logger to write the statistics to
 */
void calculate_query_stats(MtsQuerySet &query_set, bool normalized, QueryStatsLogger &logger);

#endif  // MODULES_QUERYSTATS_HPP
