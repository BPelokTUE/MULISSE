#ifndef INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_RANDOMENVCONFIGGENERATOR_HPP
#define INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_RANDOMENVCONFIGGENERATOR_HPP

#include "Index/Estimator/EnvelopeConfigGenerator/EnvConfigGeneratorParams.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/EnvelopeConfigGenerator.hpp"

class RandomEnvConfigGenerator : public IEnvelopeConfigGenerator {
   public:
    RandomEnvConfigGenerator(RandomEnvConfigGeneratorParams params);

    vec<EnvelopeParams> generate_configurations(const EnvelopeIndexProperties *env_index_params, IndexType index_type,
                                                Real index_size_limit) override;

   private:
    RandomEnvConfigGeneratorParams m_params;
};

#endif  // INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_RANDOMENVCONFIGGENERATOR_HPP
