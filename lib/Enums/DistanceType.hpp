#ifndef DISTANCE_TYPE_HPP
#define DISTANCE_TYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Types/Containers.hpp"

/** @brief Types of distance measure */
enum DistanceType { ED, MASS };

DEFINE_ENUM_CONSTS(DistanceType, DISTANCE_TYPE, false, (umap<str, DistanceType>{{"euclidean", ED}}));

#endif  // DISTANCE_TYPE_HPP
