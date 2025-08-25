#ifndef INDEX_ENVELOPEINDEX_ENVELOPETHEOPARAMESTIMATOR_HPP
#define INDEX_ENVELOPEINDEX_ENVELOPETHEOPARAMESTIMATOR_HPP

#include <random>

#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Estimator/EnvelopeParamEstimator.hpp"
#include "Util/Types/Numbers.hpp"

struct PaaDistributionInputs {
    SaxSegIndT seg_ind;
    uint length;
    uint start_pos;
    uint segment_len;
};

class EnvelopeParamTheoEstimator : public IEnvelopeParamEstimator {
   public:
    EnvelopeParams get_estimated_params(const GeneralIndexProperties &index_opts,
                                        uptr<IEnvelopeConfigGenerator> env_config_generator) override;

   private:
    Real get_paa_stdev(const PaaDistributionInputs &inputs);
};

#endif  // INDEX_ENVELOPEINDEX_ENVELOPETHEOPARAMESTIMATOR_HPP
