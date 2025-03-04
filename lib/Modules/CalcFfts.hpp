#ifndef CALC_FFTS_HPP
#define CALC_FFTS_HPP

#include "Util/typedefs.hpp"

/**
 * @brief Calculate the FFTs of the dataset
 *
 * This function calculates the FFTs of a dataset
 *
 * @param normalized Whether to normalize the dataset
 * @return int 0 if successful
 */
int calculate_ffts(bool normalized);

#endif  // CALC_FFTS_HPP
