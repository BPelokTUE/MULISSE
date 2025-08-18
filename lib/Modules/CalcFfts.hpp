#ifndef MODULES_CALCFFTS_HPP
#define MODULES_CALCFFTS_HPP

class MtsDataset;
class MtsFfts;
struct RunContext;

/**
 * @brief Calculate the FFTs of the dataset
 *
 * This function calculates the FFTs of a dataset
 *
 * @param ffts The MtsFfts object to calculate
 * @param dataset The MtsDataset to calculate the FFTs for
 * @param run_context Generic run context
 */
void calculate_ffts(MtsFfts &ffts, const MtsDataset &dataset, const RunContext &run_context);

#endif  // MODULES_CALCFFTS_HPP
