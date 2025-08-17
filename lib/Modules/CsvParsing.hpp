#ifndef MODULES_CSVPARSING_HPP
#define MODULES_CSVPARSING_HPP

#include "Util/Constants/Math.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"
#include "Util/Types/Vec.hpp"

struct MtsDataset;
struct RunContext;

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
 * @param run_context Generic run context (seed, data path, logs path)
 * @param col_sep The column separator in the CSV files
 * @param min_subs_sd The minimum standard deviation required for each valid length subsequence
 */
void create_dataset_from_csv(const MtsDataset &dataset, const vec<str> &csv_paths, uint l_min, uint l_max,
                             const RunContext &run_context, char col_sep = ',', Real min_subs_sd = DEFAULT_MIN_SUBS_SD);

#endif  // MODULES_CSVPARSING_HPP
