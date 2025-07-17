#ifndef INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP
#define INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/EnvelopeConfigGeneratorType.hpp"
#include "Enums/EnvelopeParamEstimatorType.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/EnvConfigGeneratorParams.hpp"
#include "Index/Estimator/Sampling/EstimatorSamplingParams.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

struct EstimatorParams {
    /** @brief Whether to examine whole series in EnvelopeQueryTimeParamEstimator */
    bool m_qt_examine_whole;
    /** @brief Maximum size of the index as a ratio of the dataset size. Only implemented for FlatEnvelopeIndex with LG
     * segmentation strategy other than AdaptiveMultiLGSegmentationStrategy. */
    Real m_index_size_limit;
    /** @brief Type of EnvelopeParamEstimator to use */
    EnvelopeParamEstimatorType m_param_estimator_type;
    /** @brief DistanceType used by EnvelopeQueryTimeParamEstimator */
    DistanceType m_qt_distance_type;
    /** @brief Type of configuration generator to use */
    EnvelopeConfigGeneratorType m_config_generator_type;
    /** @brief Parameters for estimator types extending EnvelopeSamplingParamEstimator */
    uptr<EstimatorSamplingParams> m_sampling_params = nullptr;
    /** @brief Parameters for the configuration generator */
    uptr<EnvConfigGeneratorParams> m_config_generator_params = nullptr;
};

#endif  // INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP
