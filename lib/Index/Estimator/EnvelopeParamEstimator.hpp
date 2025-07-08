#ifndef INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP

struct EnvelopeParams;

struct IndexOptions;

class IEnvelopeConfigGenerator;

class IEnvelopeParamEstimator {
   public:
    virtual ~IEnvelopeParamEstimator() = default;

    /**
     * @brief Get the estimated parameters for a FlatEnvelopeIndex
     * @param index_opts The IndexOptions to use
     * @param env_config_generator The configuration generator to use
     * @return Estimated index parameters
     */
    virtual EnvelopeParams get_estimated_params(const IndexOptions &index_opts,
                                                const IEnvelopeConfigGenerator *env_config_generator) = 0;
};

#endif  // INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP
