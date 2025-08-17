#ifndef MODULES_RANDOMWALK_HPP
#define MODULES_RANDOMWALK_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

struct MtsDataset;
struct RunContext;

/**
 * @brief Creates random walks and writes them to a binary file
 *
 * This function generates multiple time series of random walks and writes them
 * to a specified binary file. Run with `num_channels=1` for generating UTS.
 *
 * @param dataset Dataset to create time series for
 * @param step_sigma The standard deviation of the normal distribution used to generate the steps
 * @param zero_start If true, the random walk starts at zero; otherwise, it starts with a random value
 * @param run_context Generic run context (seed, data path, logs path)
 */
void create_random_walks(const MtsDataset &dataset, Real step_sigma, bool zero_start, const RunContext &run_context);

#endif  // MODULES_RANDOMWALK_HPP
