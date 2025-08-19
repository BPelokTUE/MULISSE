#ifndef MODULES_QUERYGEN_HPP
#define MODULES_QUERYGEN_HPP

#include <iostream>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"

struct QuerySetGenOptions;
class QuerySetLogger;

/**
 * @brief Create queries from dataset by extracting subsequences and adding noise
 *
 * Reads a dataset and creates queries by extracting subsequences of specified or random lengths at random points, from
 * random series, containing a non-empty subset of channels and adding Gaussian noise to them. The queries are saved to
 * a text file. To specify the length of the queries, either pass a list of exact lengths, in which case `num_queries`
 * queries will be generated for each of them, or a minimum and maximum length, resulting in `num_queries` queries being
 * generated with uniformly distributed length between the provided limits.
 *
 * @param query_set The query_set to generate the queries for
 * @param query_set_gen_opts Options for generating the queries
 * @param logger The logger to use for logging the query set generation
 */
void create_queries(MtsQuerySet &query_set, const QuerySetGenOptions &query_set_gen_opts, QuerySetLogger &logger);

#endif  // MODULES_QUERYGEN_HPP
