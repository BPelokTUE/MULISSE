#ifndef MODULES_INDEXING_HPP
#define MODULES_INDEXING_HPP

#include "Index/IndexOptions.hpp"

/**
 * @brief Create an index based on the specified options
 *
 * This function generates an index based on the specified options and saves it to a file. The type of index, its
 * parameters, the dataset to use and the location to save the index to are specified in the options.
 *
 * @param index_options Indexing options
 * @param sample_frac Fraction of the dataset to index. Intended for testing purposes, defaults to 1.0 (index the entire
 * dataset).
 * @param log_num_seg_per_ch Whether to log the number of segments per channel in the index.
 * @param log_num_seg_all Whether to log the number of segments for all length groups and channels in the index.
 * @return 0 on success, 1 if the dataset file could not be opened
 * */
int create_index(const IndexOptions &index_options, Real sample_frac = 1.0, bool log_num_seg_per_ch = true,
                 bool log_num_seg_all = false);

#endif  // MODULES_INDEXING_HPP
