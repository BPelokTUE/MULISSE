#ifndef MODULES_RANDOMWALK_HPP
#define MODULES_RANDOMWALK_HPP

#include "Util/Types/Numbers.hpp"

/**
 * @brief Creates random walks and writes them to a binary file
 *
 * This function generates multiple time series of random walks and writes them
 * to a specified binary file. Run with `num_channels=1` for generating UTS.
 *
 * @param step_sigma The standard deviation of the normal distribution used to generate the steps
 * @param zero_start If true, the random walk starts at zero; otherwise, it starts with a random value
 * @param seed The seed for the random number generator
 * @return 0 on success, 1 if the file could not be created.
 */
int create_random_walks(Real step_sigma, bool zero_start, uint seed);

#endif  // MODULES_RANDOMWALK_HPP
