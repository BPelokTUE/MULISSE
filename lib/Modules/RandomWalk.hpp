#ifndef RANDOMWALK_HPP
#define RANDOMWALK_HPP

#include "Util/typedefs.hpp"

/**
 * @brief Creates random walks and writes them to a binary file
 *
 * This function generates multiple time series of random walks and writes them
 * to a specified binary file. Run with `num_channels=1` for generating UTS.
 *
 * @param rw_noise The standard deviation of the normal distribution used to generate noise
 * @param zero_start If true, the random walk starts at zero; otherwise, it starts with a random value
 * @param num_series The number of time series to generate
 * @param series_len The length of each time series
 * @param num_channels The number of channels in each time series
 * @param seed The seed for the random number generator
 * @return 0 on success, 1 if the dataset file already exists, 2 if the file could not be created.
 */
int create_random_walks(float rw_noise, bool zero_start, uint num_series, uint series_len, uint num_channels, int seed);

#endif  // RANDOMWALK_HPP
