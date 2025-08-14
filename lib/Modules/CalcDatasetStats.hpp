#ifndef MODULES_DATASETSTATS_HPP
#define MODULES_DATASETSTATS_HPP

#include "Util/Types/Numbers.hpp"

class MtsDataset;

/**
 * @brief Calculate statistics of a dataset
 * @param num_lags Number of lags to calculate for autocorrelation and total variance
 */
void calculate_dataset_stats(MtsDataset &dataset, uint num_lags);

#endif  // MODULES_DATASETSTATS_HPP
