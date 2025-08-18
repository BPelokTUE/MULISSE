#ifndef UTIL_ARTEFACTS_OPTIONS_RANDOMWALKGENOPTIONS_HPP
#define UTIL_ARTEFACTS_OPTIONS_RANDOMWALKGENOPTIONS_HPP

#include "Util/Types/Numbers.hpp"

/** @brief Options for generating  */
struct RandomWalkGenOptions {
    /** @brief Whether to start the random walk at zero */
    bool m_zero_start;
    /** @brief The random seed to use */
    int m_seed = 0;
    /** @brief The standard deviation of the Gaussian noise to add at each step */
    Real m_step_sd;
};

#endif  // UTIL_ARTEFACTS_OPTIONS_RANDOMWALKGENOPTIONS_HPP
