#ifndef INDEX_ESTIMATOR_FLATENVELOPESIZEESTIMATOR_HPP
#define INDEX_ESTIMATOR_FLATENVELOPESIZEESTIMATOR_HPP

#include "Util/RunSettings/LengthProperties.hpp"
#include "Util/Types/Numbers.hpp"

class ILengthGroupSegmentationStrategy;

class FlatEnvelopeSizeEstimator {
   public:
    /**
     * @brief Constructor for FlatEnvelopeSizeEstimator
     * @param length_props The length properties used
     * @param lg_segmentation_strategy The length group segmentation strategy used
     * @param num_segments The number of segments per channel, used if lg_segmentation_strategy is nullptr
     * */
    FlatEnvelopeSizeEstimator(LengthProperties length_props,
                              const ILengthGroupSegmentationStrategy *lg_segmentation_strategy = nullptr,
                              SaxSegIndT num_segments = 0);

    /**
     * @brief Estimate the size of a FlatEnvelopeIndex
     * @param pos_per_env The number of positions per envelope used
     * @param add_entry_vec_size Whether to take the size of the entry vector into account
     * @return Estimated size of the FlatEnvelopeIndex in bytes
     */
    size_t get_estimated_flat_envelope_size(uint pos_per_env, bool add_entry_vec_size = true);

    /**
     * @brief Get the maximum number of positions per envelope for a FlatEnvelopeIndex
     * @param index_size_limit The maximum size of the index as a ratio of the dataset size
     * @return Maximum number of positions per envelope
     */
    uint get_max_pos_per_env(Real index_size_limit);

   private:
    /**
     * @brief Get the overhead size of the FlatEnvelopeIndex
     * @return Overhead size in bytes
     */
    size_t get_overhead_size();

    const ILengthGroupSegmentationStrategy *m_lg_segmentation_strategy;
    LengthProperties m_length_props;
    uint m_segment_len;
};

#endif  // INDEX_ESTIMATOR_FLATENVELOPESIZEESTIMATOR_HPP
