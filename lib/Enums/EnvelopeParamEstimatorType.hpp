#ifndef ENUMS_ENVELOPEPARAMESTIMATORTYPE_HPP
#define ENUMS_ENVELOPEPARAMESTIMATORTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Types of EnvelopeParamEstimator */
enum EnvelopeParamEstimatorType { THEORETICAL, MIN_DIST, QUERY_TIME, NO_EST };

DEFINE_ENUM_CONSTS(EnvelopeParamEstimatorType, ENV_PARAM_ESTIMATOR_TYPE, false,
                   (umap<str, EnvelopeParamEstimatorType>{
                       {"theo", THEORETICAL}, {"time", QUERY_TIME}, {"none", NO_EST}}));

constexpr std::array SAMPLING_ESTIMATOR_TYPES{MIN_DIST, QUERY_TIME};

#endif  // ENUMS_ENVELOPEPARAMESTIMATORTYPE_HPP
