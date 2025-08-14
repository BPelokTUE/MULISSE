#ifndef MODULES_RANDOMWALK_HPP
#define MODULES_RANDOMWALK_HPP

#include "Util/Types/Numbers.hpp"

struct MtsDataset;

/**
 * @brief Creates random walks and writes them to a binary file
 *
 * This function generates multiple time series of random walks and writes them
 * to a specified binary file. Run with `num_channels=1` for generating UTS.
 *
 * @param dataset Dataset to create time series for
 * @param step_sigma The standard deviation of the normal distribution used to generate the steps
 * @param zero_start If true, the random walk starts at zero; otherwise, it starts with a random value
 * @param seed The seed for the random number generator
 */
void create_random_walks(const MtsDataset &dataset, Real step_sigma, bool zero_start, uint seed);

#endif  // MODULES_RANDOMWALK_HPP
