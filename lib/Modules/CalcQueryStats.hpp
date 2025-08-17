#ifndef MODULES_QUERYSTATS_HPP
#define MODULES_QUERYSTATS_HPP

class MtsQuerySet;
struct RunContext;

/**
 * @brief Calculate statistics of a query set
 *
 * This function computes statistics for a query set, such as the minimum, maximum, mean and standard deviation of each
 * query to subsequences in the dataset (these can be used to calculate relative contrast for example).
 *
 * @param dataset The MtsDataset the query set is based on
 * @param query_set The MtsQuerySet to calculate statistics for
 * @param run_context Generic run context ()
 */
int calculate_query_stats(const MtsDataset &dataset, const MtsQuerySet &query_set, const RunContext &run_context);

#endif  // MODULES_QUERYSTATS_HPP
