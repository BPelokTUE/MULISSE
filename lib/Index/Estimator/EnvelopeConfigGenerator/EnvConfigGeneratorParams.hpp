#ifndef INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVCONFIGGENERATORPARAMS_HPP
#define INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVCONFIGGENERATORPARAMS_HPP

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

struct EnvConfigGeneratorParams {
    virtual ~EnvConfigGeneratorParams() = default;
};

struct RandomEnvConfigGeneratorParams : public EnvConfigGeneratorParams {
    /** @brief Maximum number of segments for the generated configurations */
    SaxSegIndT m_num_segments_max;
    /** @brief Minimum number of segments for the generated configurations */
    SaxSegIndT m_num_segments_min;
    /** @brief Number of configurations to generate */
    uint m_num_configs;
    /** @brief Random seed */
    uint m_seed;

    RandomEnvConfigGeneratorParams(uint num_configs = 100, uint seed = 0, SaxSegIndT num_segments_max = 64,
                                   SaxSegIndT num_segments_min = 1)
        : m_num_segments_min(num_segments_min),
          m_num_segments_max(num_segments_max > 0 ? num_segments_max : 64),
          m_num_configs(num_configs),
          m_seed(seed) {}
};

#endif  // INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVCONFIGGENERATORPARAMS_HPP
