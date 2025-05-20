#ifndef MODULES_DATASETSTATS_HPP
#define MODULES_DATASETSTATS_HPP

#include "Util/Types/Numbers.hpp"

/**
 * @brief Calculate statistics of a dataset
 * @param num_lags Number of lags to calculate for autocorrelation and total variance
 * @return 0 on success, 1 if the dataset does not exist
 * */
int calculate_dataset_stats(uint num_lags);

#endif  // MODULES_DATASETSTATS_HPP
