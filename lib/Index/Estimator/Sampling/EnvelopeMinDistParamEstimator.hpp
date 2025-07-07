#ifndef INDEX_ESTIMATOR_SAMPLING_ENVELOPEMINDISTPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_SAMPLING_ENVELOPEMINDISTPARAMESTIMATOR_HPP

#include "Index/Estimator/Sampling/EnvelopeSamplingParamEstimator.hpp"
#include "Index/Estimator/Sampling/EstimatorSamplingParams.hpp"

class EnvelopeMinDistParamEstimator : public EnvelopeSamplingParamEstimator {
   public:
    /**
     * @brief Constructor for EnvelopeParamMinDistEstimator
     * @param index_opts The initial index options to use for the FlatEnvelopeIndex
     */
    EnvelopeMinDistParamEstimator(const IndexOptions &index_opts);

   protected:
    void update_queries(std::stringstream &query_stream, uint num_queries) override;

    Real get_config_score(vec<vec<IndexEntry<Envelope>>> &&entries, const IndexOptions &index_opts,
                          const LengthProperties &length_props,
                          const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) override;

   private:
    EnvelopeParams m_estimated_params;
    vec<vec<vec<Real>>> m_query_accs;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_ENVELOPEMINDISTPARAMESTIMATOR_HPP
