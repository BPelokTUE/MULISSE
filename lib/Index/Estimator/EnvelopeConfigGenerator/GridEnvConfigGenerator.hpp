#ifndef INDEX_ESTIMATOR_CONFIGGENERATOR_DUMMYCONFIGGENERATOR_HPP
#define INDEX_ESTIMATOR_CONFIGGENERATOR_DUMMYCONFIGGENERATOR_HPP

#include "Index/Estimator/EnvelopeConfigGenerator/EnvelopeConfigGenerator.hpp"

class GridEnvConfigGenerator : public IEnvelopeConfigGenerator {
   public:
    vec<EnvelopeParams> generate_configurations(const EnvelopeIndexProperties *env_index_params,
                                                SearchMethodType index_type, Real index_size_limit) override;
};

#endif  // INDEX_ESTIMATOR_CONFIGGENERATOR_DUMMYCONFIGGENERATOR_HPP
