#ifndef INDEX_ESTIMATOR_FLATENVELOPEMINDISTPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_FLATENVELOPEMINDISTPARAMESTIMATOR_HPP

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"

struct IndexOptions;

class FlatEnvelopeMinDistParamEstimator : public IFlatEnvelopeParamEstimator {
   public:
    /**
     * @brief Constructor for FlatEnvelopeParamTheoEstimator
     * @param opts The initial index options to use for the FlatEnvelopeIndex
     */
    FlatEnvelopeMinDistParamEstimator(const IndexOptions &opts);

    FlatEnvelopeParams get_estimated_params() override;

   private:
    FlatEnvelopeParams m_estimated_params;
};

#endif  // INDEX_ESTIMATOR_FLATENVELOPEMINDISTPARAMESTIMATOR_HPP
