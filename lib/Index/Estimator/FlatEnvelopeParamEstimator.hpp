#ifndef INDEX_ESTIMATOR_FLATENVELOPEPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_FLATENVELOPEPARAMESTIMATOR_HPP

struct FlatEnvelopeParams;

class IFlatEnvelopeParamEstimator {
   public:
    virtual ~IFlatEnvelopeParamEstimator() = default;

    /**
     * @brief Get the estimated parameters for a FlatEnvelopeIndex
     * @return Estimated index parameters
     */
    virtual FlatEnvelopeParams get_estimated_params() = 0;
};

#endif  // INDEX_ESTIMATOR_FLATENVELOPEPARAMESTIMATOR_HPP
