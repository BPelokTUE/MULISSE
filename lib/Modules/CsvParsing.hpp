#ifndef MODULES_CSVPARSING_HPP
#define MODULES_CSVPARSING_HPP

#include "Util/Constants/Math.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"
#include "Util/Types/Vec.hpp"

struct MtsDataset;

/**
 * @brief Create a binary dataset from a CSV file
 *
 * Creates a binary dataset from a CSV file. Each row in the file is considered one channel. If a series is too short,
 * it is discarded, and if it is too long, it is truncated. The length and number of channels of the series are set in
 * RunSettings.
 *
 * @param dataset The MTS dataset to create
 * @param csv_path The list of csv file paths in the order of channels
 * @param l_min Discard time series where the standard deviation is too low in any subsequence of length between `l_min`
 * and `l_max
 * @param l_max Discard time series where the standard deviation is too low in any subsequence of length between `l_min`
 * and `l_max
 * @param col_sep The column separator in the CSV files
 * @param min_subs_sd The minimum standard deviation required for each valid length subsequence
 * @param seed The seed for the random number generator
 * @param data_path The path to the data directory
 * @param logs_path The path to the logs directory
 */
void create_dataset_from_csv(const MtsDataset &dataset, const vec<str> &csv_paths, uint l_min, uint l_max,
                             char col_sep = ',', Real min_subs_sd = DEFAULT_MIN_SUBS_SD, uint seed = 0,
                             const str &data_path = DEFAULT_DATA_PATH, const str &logs_path = DEFAULT_LOGS_PATH);

#endif  // MODULES_CSVPARSING_HPP
