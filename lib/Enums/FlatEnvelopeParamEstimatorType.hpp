#ifndef ENUMS_FLATENVELOPEPARAMESTIMATORTYPE_HPP
#define ENUMS_FLATENVELOPEPARAMESTIMATORTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Types of FlatEnvelopeParamEstimator */
enum FlatEnvelopeParamEstimatorType { THEORETICAL, MIN_DIST, QUERY_TIME };

DEFINE_ENUM_CONSTS(FlatEnvelopeParamEstimatorType, FLAT_ENVELOPE_PARAM_ESTIMATOR_TYPE, false,
                   (umap<str, FlatEnvelopeParamEstimatorType>{{"theo", THEORETICAL}, {"time", QUERY_TIME}}));

#endif  // ENUMS_FLATENVELOPEPARAMESTIMATORTYPE_HPP
