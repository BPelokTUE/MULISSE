#ifndef MODULES_DATASETSTATS_HPP
#define MODULES_DATASETSTATS_HPP

#include "Util/Types/Numbers.hpp"

class MtsDataset;
class DatasetStatsLogger;

/**
 * @brief Calculate statistics of a dataset
 * @param dataset The MtsDataset to calculate statistics for
 * @param num_lags Number of lags to calculate for autocorrelation and total variance
 * @param logger Logger to log the dataset statistics
 */
void calculate_dataset_stats(MtsDataset &dataset, uint num_lags, DatasetStatsLogger &logger);

#endif  // MODULES_DATASETSTATS_HPP
