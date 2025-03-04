#ifndef SEARCHING_HPP
#define SEARCHING_HPP

#include "Search/Options/SearchOptions.hpp"

/**
 * @brief Execute similarity search
 * @param opts Search options
 * @return 0 if successful, 1 otherwise
 */
int search(const SearchOptions &opts);

#endif  // SEARCHING_HPP
