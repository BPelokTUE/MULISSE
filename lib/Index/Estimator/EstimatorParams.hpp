#ifndef INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP
#define INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP

#include "Util/Types/Numbers.hpp"

struct EstimatorParams {
    /** @brief Whether to estimate the optimal parameters for FlatEnvelopeIndex given a size limit */
    bool m_estimate_parameters;
    /** @brief Maximum size of the index as a ratio of the dataset size. Only implemented for FlatEnvelopeIndex with LG
     * segmentation strategy other than AdaptiveMultiLGSegmentationStrategy. */
    Real m_index_size_limit;
};

#endif  // INDEX_ESTIMATOR_ESTIMATORPARAMS_HPP
