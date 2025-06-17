#ifndef INDEX_ENVELOPEINDEX_FLAT_FLATENVELOPETHEOPARAMESTIMATOR_HPP
#define INDEX_ENVELOPEINDEX_FLAT_FLATENVELOPETHEOPARAMESTIMATOR_HPP

#include <random>

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"
#include "Util/Types/Numbers.hpp"

struct IndexOptions;

struct PaaDistributionInputs {
    SaxSegIndT seg_ind;
    uint length;
    uint start_pos;
    uint segment_len;
};

class FlatEnvelopeParamTheoEstimator : public IFlatEnvelopeParamEstimator {
   public:
    /**
     * @brief Constructor for FlatEnvelopeParamTheoEstimator
     * @param opts The initial index options to use for the FlatEnvelopeIndex
     */
    FlatEnvelopeParamTheoEstimator(const IndexOptions &opts);

    FlatEnvelopeParams get_estimated_params() override;

   private:
    Real get_paa_stdev(const PaaDistributionInputs &inputs);

    FlatEnvelopeParams m_estimated_params;
};

#endif  // INDEX_ENVELOPEINDEX_FLAT_FLATENVELOPETHEOPARAMESTIMATOR_HPP
