#ifndef MODULES_INDEXSTATS_HPP
#define MODULES_INDEXSTATS_HPP

#include <type_traits>

#include "Enums/ArchiveType.hpp"
#include "Enums/IndexType.hpp"
#include "Index/FinalizedIndex.hpp"
#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Util/Logging/IndexStatsLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Pointers.hpp"

/**
 * @brief Calculate index statistics
 * @param method_type The type of index to use
 * @param num_l_groups The number of length groups to use
 * @param index_format The format of the index
 * @param separate_segment_stats Whether to calculate segment statistics for each segment separately
 */
int calculate_index_stats(IndexType method_type, uint num_l_groups, ArchiveType index_format,
                          bool separate_segment_stats = false);

#endif  // MODULES_INDEXSTATS_HPP
