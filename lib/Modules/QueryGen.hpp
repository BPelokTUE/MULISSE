#ifndef QUERY_GEN_HPP
#define QUERY_GEN_HPP

#include <string>
#include <vector>

#include "typedefs.hpp"

/**
 * @brief Create queries from dataset by extracting subsequences and adding noise
 *
 * This function reads a dataset from a binary file and creates queries by extracting subsequences
 * of specified lengths at random points, from random series, containing a random non-empty subset
 * of channels and adding noise to them. The queries are saved to a text file.
 *
 * @param dataset_path Path to dataset
 * @param query_path Path to save queries
 * @param noise Noise to add to the queries
 * @param series_len Length of the series
 * @param num_channels Number of channels in the dataset
 * @param num_queries Number of queries to generate
 * @param lengths Lengths of the queries
 * @return 0 on success, 1 if dataset does not exist, 2 if query already exists, 3 if query could not be created
 */
int create_queries(std::string dataset_path, std::string query_path, float noise, unsigned num_series,
                   unsigned num_channels, unsigned num_queries, vec<unsigned> lengths);

#endif  // QUERY_GEN_HPP
