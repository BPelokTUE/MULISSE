#ifndef INDEX_ESTIMATOR_ENVELOPEONLYGAMMAESTIMATOR_HPP
#define INDEX_ESTIMATOR_ENVELOPEONLYGAMMAESTIMATOR_HPP

#include "Index/Estimator/EnvelopeParamEstimator.hpp"

class EnvelopeOnlyGammaEstimator : public IEnvelopeParamEstimator {
    EnvelopeParams m_envelope_params;
    size_t m_index_size_limit;

   public:
    EnvelopeOnlyGammaEstimator(const EnvelopeParams &envelope_params, size_t index_size_limit);

    EnvelopeParams get_estimated_params(const GeneralIndexProperties &index_opts,
                                        uptr<IEnvelopeConfigGenerator> env_config_generator) override;
};

#endif  // INDEX_ESTIMATOR_ENVELOPEONLYGAMMAESTIMATOR_HPP
