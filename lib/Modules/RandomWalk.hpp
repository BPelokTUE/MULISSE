#ifndef MODULES_RANDOMWALK_HPP
#define MODULES_RANDOMWALK_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

struct MtsDataset;
struct RandomWalkGenOptions;
class DatasetLogger;

/**
 * @brief Creates random walks and writes them to a binary file
 *
 * This function generates multiple time series of random walks and writes them
 * to a specified binary file. Run with `num_channels=1` for generating UTS.
 *
 * @param dataset Dataset to create time series for
 * @param rw_gen_opts Options for generating the random walk dataset
 * @param logger Logger to log the dataset generation
 */
void create_random_walks(MtsDataset &dataset, const RandomWalkGenOptions &rw_gen_opts, DatasetLogger &logger);

#endif  // MODULES_RANDOMWALK_HPP
