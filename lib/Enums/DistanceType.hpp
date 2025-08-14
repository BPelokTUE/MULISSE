#ifndef ENUMS_DISTANCE_TYPE_HPP
#define ENUMS_DISTANCE_TYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Types of distance measure */
enum DistanceType { ED, MASS };

DEFINE_ENUM_CONSTS(DistanceType, DISTANCE_TYPE, false, (umap<str, DistanceType>{{"euclidean", ED}}));

#endif  // ENUMS_DISTANCE_TYPE_HPP
