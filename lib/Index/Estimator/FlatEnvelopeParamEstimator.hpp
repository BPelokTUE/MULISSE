#ifndef INDEX_ENVELOPEINDEX_FLAT_FLATENVELOPEPARAMESTIMATOR_HPP
#define INDEX_ENVELOPEINDEX_FLAT_FLATENVELOPEPARAMESTIMATOR_HPP

#include <random>

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"

struct IndexOptions;

struct PaaDistributionInputs {
    SaxSegIndT seg_ind;
    uint length;
    uint start_pos;
    uint segment_len;
};

class FlatEnvelopeParamEstimator {
   public:
    /**
     * @brief Constructor for FlatEnvelopeParamEstimator
     * @param opts The initial index options to use for the FlatEnvelopeIndex
     */
    FlatEnvelopeParamEstimator(const IndexOptions &opts);

    /**
     * @brief Get the estimated parameters for a FlatEnvelopeIndex
     * @return Estimated index parameters
     */
    FlatEnvelopeParams get_estimated_params();

   private:
    Real get_paa_stdev(const PaaDistributionInputs &inputs);

    FlatEnvelopeParams m_estimated_params;
};

#endif  // INDEX_ENVELOPEINDEX_FLAT_FLATENVELOPEPARAMESTIMATOR_HPP
