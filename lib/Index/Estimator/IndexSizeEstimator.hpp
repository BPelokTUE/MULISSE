#ifndef UTIL_STATS_INDEXSIZEESTIMATOR_HPP
#define UTIL_STATS_INDEXSIZEESTIMATOR_HPP

#include "Util/Types/Numbers.hpp"

class ILengthGroupSegmentationStrategy;

/**
 * @brief Get the overhead size of the FlatEnvelopeIndex
 * @return Overhead size in bytes
 */
size_t get_overhead_size();

/**
 * @brief Estimate the size of a FlatEnvelopeIndex
 * @param pos_per_env The number of positions per envelope used
 * @param lg_segmentation_strategy The length group segmentation strategy to used
 * @param num_segments The number of segments per channel, used if lg_segmentation_strategy is nullptr
 * @param add_entry_vec_size Whether to take the size of the entry vector into account
 * @return Estimated size of the FlatEnvelopeIndex in bytes
 */
size_t get_estimated_flat_envelope_size(uint pos_per_env,
                                        const ILengthGroupSegmentationStrategy *lg_segmentation_strategy = nullptr,
                                        SaxSegIndT num_segments = 0, bool add_entry_vec_size = true);

/**
 * @brief Get the maximum number of positions per envelope for a FlatEnvelopeIndex
 * @param index_size_limit The maximum size of the index as a ratio of the dataset size
 * @param lg_segmentation_strategy The length group segmentation strategy to used
 * @param num_segments The number of segments per channel, used if lg_segmentation_strategy is nullptr
 * @return Maximum number of positions per envelope
 */
uint get_max_pos_per_env(Real index_size_limit,
                         const ILengthGroupSegmentationStrategy *lg_segmentation_strategy = nullptr,
                         SaxSegIndT num_segments = 0);

#endif  // UTIL_STATS_INDEXSIZEESTIMATOR_HPP
