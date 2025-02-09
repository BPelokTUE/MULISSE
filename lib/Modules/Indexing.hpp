#ifndef INDEXING_HPP
#define INDEXING_HPP

#include "Util/typedefs.hpp"
#include "Search/Options/IndexOptions.hpp"

/**
 * @brief Create an index based on the specified options
 *
 * This function generates an index based on the specified options and saves it to a file.  The type of index, its
 * parameters, the dataset to use and the location to save the index to are specified in the options.
 *
 * @param index_options Indexing options
 * @return 0 on success, 1 if the dataset file could not be opened
 * */
int create_index(const IndexOptions &index_options);

#endif  // INDEXING_HPP
