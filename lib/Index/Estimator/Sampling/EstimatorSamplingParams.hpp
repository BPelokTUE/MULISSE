#ifndef INDEX_ESTIMATOR_SAMPLING_ESTIMATORSAMPLINGPARAMS_HPP
#define INDEX_ESTIMATOR_SAMPLING_ESTIMATORSAMPLINGPARAMS_HPP

#include "Util/Types/Numbers.hpp"

struct EstimatorSamplingParams {
    uint m_seed;
    uint m_last_ind_step;
    uint m_first_ind_step;
    uint m_num_queries;
    Real m_sample_frac;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_ESTIMATORSAMPLINGPARAMS_HPP
