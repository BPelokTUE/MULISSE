#ifndef MODULES_QUERYGEN_HPP
#define MODULES_QUERYGEN_HPP

#include <iostream>

#include "Search/QueryGenOptions.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"

struct RunContext;

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
 * @param dataset The dataset to generate the queries from
 * @param query_set_gen_opts Options for generating the queries
 * @param run_context Generic run context
 */
void create_queries(MtsQuerySet &query_set, MtsDataset &dataset, const QuerySetGenOptions &query_set_gen_opts,
                    const RunContext &run_context);

/**
 * @brief Generate queries from the data stream and write them to the query stream
 * @param data_is Input stream containing the dataset
 * @param query_os Output stream to write the queries to
 * @param dataset_props Properties of the MtsDataset
 * @param query_set_props Properties of the MtsQuerySet
 * @param query_set_gen_opts Options for generating the queries
 * @param series_inds Optional vector of series indices to use for generating queries. If empty, all series will be
 * used.
 */
void generate_queries(std::istream &data_is, std::ostream &query_os, const MtsDatasetProperties &dataset_props,
                      const MtsQuerySetProperties &query_set_props, const QuerySetGenOptions &query_set_gen_opts,
                      const vec<uint> &series_inds = {});

#endif  // MODULES_QUERYGEN_HPP
