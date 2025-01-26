#ifndef INDEXING_HPP
#define INDEXING_HPP

#include "typedefs.hpp"

#include "Search/IndexOptions.hpp"

int create_index(std::string dataset_path, unsigned series_len, unsigned num_channels, IndexOptions index_options);

#endif  // INDEXING_HPP
