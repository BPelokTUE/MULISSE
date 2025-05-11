#ifndef MODULES_CSVPARSING_HPP
#define MODULES_CSVPARSING_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/**
 * @brief Create a binary dataset from a CSV file
 *
 * Creates a binary dataset from a CSV file. Each row in the file is considered one channel. If a series is too short,
 * it is discarded, and if it is too long, it is truncated. The length and number of channels of the series are set in
 * RunSettings.
 *
 * @param csv_path The list of csv file paths in the order of channels
 * @param num_series The maximum number of series to generate
 * @param l_min Discard time series where the standard deviation is too low in any subsequence of length between `l_min`
 * and `l_max
 * @param l_max Discard time series where the standard deviation is too low in any subsequence of length between `l_min`
 * and `l_max
 * @param seed The seed for the random number generator
 * @param col_sep The column separator in the CSV files
 */
int create_dataset_from_csv(const vec<str> &csv_paths, uint num_series, uint l_min, uint l_max, uint seed = 0,
                            char col_sep = ',');

#endif  // MODULES_CSVPARSING_HPP
