#ifndef INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP
#define INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP

#include "Enums/FlatEnvelopeParamEstimatorType.hpp"
#include "Index/Estimator/Sampling/EstimatorSamplingParams.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

struct EstimatorParams {
    /** @brief Maximum size of the index as a ratio of the dataset size. Only implemented for FlatEnvelopeIndex with LG
     * segmentation strategy other than AdaptiveMultiLGSegmentationStrategy. */
    Real m_index_size_limit;
    /** @brief Type of FlatEnvelopeParamEstimator to use */
    FlatEnvelopeParamEstimatorType m_param_estimator_type;
    /** @brief Parameters for estimator types extending FlatEnvelopeSamplingParamEstimator */
    uptr<EstimatorSamplingParams> m_sampling_params = nullptr;
};

#endif  // INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP
