#ifndef ENUMS_LENGTHGROUPSEGMENTATIONSTRATEGYTYPE_HPP
#define ENUMS_LENGTHGROUPSEGMENTATIONSTRATEGYTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enum for ILengthGroupSegmentationStrategy implementations */
enum LengthGroupSegmentationStrategyType { SINGLE, MULTI, ADAPTIVE_MULTI };

DEFINE_ENUM_CONSTS_NO_EXTRA(LengthGroupSegmentationStrategyType, LENGTH_GROUP_SEGMENTATION_STRATEGY, false);

#endif  // ENUMS_LENGTHGROUPSEGMENTATIONSTRATEGYTYPE_HPP
