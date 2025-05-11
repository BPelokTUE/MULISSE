#ifndef ENUMS_SEGMENTATIONSTRATEGYTYPE_HPP
#define ENUMS_SEGMENTATIONSTRATEGYTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enum for ISegmentationStrategy implementations */
enum SegmentationStrategyType { UNIFORM, ADAPTIVE };

DEFINE_ENUM_CONSTS(SegmentationStrategyType, SEGMENTATION_STRATEGY, false,
                   (umap<str, SegmentationStrategyType>{{"equi_width", UNIFORM}, {"equi_depth", ADAPTIVE}}));

#endif  // ENUMS_SEGMENTATIONSTRATEGYTYPE_HPP
