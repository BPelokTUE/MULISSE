#ifndef MODULES_CALCFFTS_HPP
#define MODULES_CALCFFTS_HPP

class MtsDataset;
class MtsDatasetFfts;
struct RunContext;
class FftsLogger;

/**
 * @brief Calculate the FFTs of the dataset
 *
 * This function calculates the FFTs of a dataset
 *
 * @param ffts The MtsFfts object to calculate
 * @param logger The FftsLogger to use for logging
 */
void calculate_ffts(MtsDatasetFfts &ffts, FftsLogger &logger);

#endif  // MODULES_CALCFFTS_HPP
