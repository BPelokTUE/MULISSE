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
 * @param csv_gen_opts Options for generating the dataset from CSV files
 * @param logger Logger to log dataset settings
 */
void create_dataset_from_csv(MtsDataset &dataset, const CsvDatasetGenOptions &csv_gen_opts,
                             const DatasetLogger &logger);

#endif  // MODULES_CSVPARSING_HPP
