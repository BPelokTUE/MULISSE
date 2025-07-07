#ifndef INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP

struct EnvelopeParams;

class IEnvelopeParamEstimator {
   public:
    virtual ~IEnvelopeParamEstimator() = default;

    /**
     * @brief Get the estimated parameters for a FlatEnvelopeIndex
     * @return Estimated index parameters
     */
    virtual EnvelopeParams get_estimated_params() = 0;
};

#endif  // INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP
