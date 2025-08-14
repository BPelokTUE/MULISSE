#ifndef MODULES_QUERYGEN_HPP
#define MODULES_QUERYGEN_HPP

#include <iostream>

#include "Search/QueryGenOptions.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQueryset.hpp"

/**
 * @brief Create queries from dataset by extracting subsequences and adding noise
 *
 * Reads a dataset and creates queries by extracting subsequences of specified or random lengths at random points, from
 * random series, containing a non-empty subset of channels and adding Gaussian noise to them. The queries are saved to
 * a text file. To specify the length of the queries, either pass a list of exact lengths, in which case `num_queries`
 * queries will be generated for each of them, or a minimum and maximum length, resulting in `num_queries` queries being
 * generated with uniformly distributed length between the provided limits.
 *
 * @param dataset The dataset to generate the queries from
 * @param queryset The queryset to generate the queries for
 * @param opts Options for generating the queries
 */
void create_queries(MtsDataset &dataset, MtsQueryset &queryset, QuerysetGenOptions opts);

/**
 * @brief Generate queries from the data stream and write them to the query stream
 * @param data Input stream containing the dataset
 * @param query Output stream to write the queries to
 * @param opts Options for generating the queries
 * @param series_inds Optional vector of series indices to use for generating queries. If empty, all series will be
 * used.
 */
void generate_queries(std::istream &data, std::ostream &query, const QuerysetGenOptions &opts,
                      const vec<uint> &series_inds = {});

#endif  // MODULES_QUERYGEN_HPP
