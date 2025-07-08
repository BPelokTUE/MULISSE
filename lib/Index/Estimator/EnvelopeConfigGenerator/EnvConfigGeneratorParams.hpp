#ifndef INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVCONFIGGENERATORPARAMS_HPP
#define INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVCONFIGGENERATORPARAMS_HPP

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

struct EnvConfigGeneratorParams {
    virtual ~EnvConfigGeneratorParams() = default;
};

struct RandomEnvConfigGeneratorParams : public EnvConfigGeneratorParams {
    /** @brief Minimum number of segments for the generated configurations */
    SaxSegIndT m_num_segments_min;
    /** @brief Maximum number of segments for the generated configurations */
    SaxSegIndT m_num_segments_max;
    /** @brief Number of configurations to generate */
    uint m_num_configs;
    /** @brief Random seed */
    uint m_seed;
    /** @brief Minimum number of lengths per group for the generated configurations */
    Real m_l_per_group_ratio_min;
    /** @brief Maximum number of lengths per group for the generated configurations */
    Real m_l_per_group_ratio_max;

    RandomEnvConfigGeneratorParams(uint num_configs = 100, uint seed = 0, SaxSegIndT num_segments_min = 4,
                                   SaxSegIndT num_segments_max = 32, Real l_per_group_ratio_min = R(0.025),
                                   uint l_per_group_ratio_max = R(1.0))
        : m_num_segments_min(num_segments_min),
          m_num_segments_max(num_segments_max),
          m_num_configs(num_configs),
          m_seed(seed),
          m_l_per_group_ratio_min(l_per_group_ratio_min),
          m_l_per_group_ratio_max(l_per_group_ratio_max) {}
};

#endif  // INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVCONFIGGENERATORPARAMS_HPP
