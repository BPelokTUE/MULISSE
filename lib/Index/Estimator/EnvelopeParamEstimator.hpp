#ifndef INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP

#include "Util/Types/Pointers.hpp"

struct EnvelopeParams;

struct GeneralIndexProperties;

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
    virtual EnvelopeParams get_estimated_params(const GeneralIndexProperties &index_opts,
                                                uptr<IEnvelopeConfigGenerator> env_config_generator) = 0;
};

#endif  // INDEX_ESTIMATOR_ENVELOPEPARAMESTIMATOR_HPP
