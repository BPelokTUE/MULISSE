#include "Util/typedefs.hpp"

/**
 * @brief Create a binary dataset from a CSV file
 *
 * Creates a binary dataset from a CSV file. Each row in the file is considered one channel. If a series is too short,
 * it is discarded, and if it is too long, it is truncated. The length and number of channels of the series are set in
 * RunSettings.
 *
 * @param csv_path The list of csv file paths in the order of channels
 */
int create_dataset_from_csv(const vec<str> &csv_paths, const char col_sep = ',');
