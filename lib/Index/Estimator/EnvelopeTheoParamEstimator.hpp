#ifndef INDEX_ENVELOPEINDEX_ENVELOPETHEOPARAMESTIMATOR_HPP
#define INDEX_ENVELOPEINDEX_ENVELOPETHEOPARAMESTIMATOR_HPP

#include <random>

#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Estimator/EnvelopeParamEstimator.hpp"
#include "Util/Types/Numbers.hpp"

struct IndexOptions;

struct PaaDistributionInputs {
    SaxSegIndT seg_ind;
    uint length;
    uint start_pos;
    uint segment_len;
};

class EnvelopeParamTheoEstimator : public IEnvelopeParamEstimator {
   public:
    /**
     * @brief Constructor for EnvelopeParamTheoEstimator
     * @param opts The initial index options to use for the FlatEnvelopeIndex
     */
    EnvelopeParamTheoEstimator(const IndexOptions &opts);

    EnvelopeParams get_estimated_params() override;

   private:
    Real get_paa_stdev(const PaaDistributionInputs &inputs);

    EnvelopeParams m_estimated_params;
};

#endif  // INDEX_ENVELOPEINDEX_ENVELOPETHEOPARAMESTIMATOR_HPP
