#ifndef INDEX_ESTIMATOR_SAMPLING_ENVELOPEQUERYTIMEPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_SAMPLING_ENVELOPEQUERYTIMEPARAMESTIMATOR_HPP

#include "Index/Estimator/Sampling/EnvelopeSamplingParamEstimator.hpp"
#include "Index/Estimator/Sampling/EstimatorSamplingParams.hpp"

class EnvelopeQueryTimeParamEstimator : public EnvelopeSamplingParamEstimator {
   protected:
    EnvelopeParams get_estimated_params(const IndexOptions &index_opts,
                                        const IEnvelopeConfigGenerator *env_config_generator) override;

    void update_queries(std::stringstream &query_stream, uint num_queries) override;

    Real get_config_score(vec<vec<IndexEntry<Envelope>>> &&entries, const IndexOptions &config_opts,
                          const LengthProperties &length_props,
                          const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) override;

   private:
    EnvelopeParams m_estimated_params;
    vec<vec<vec<Real>>> m_queries;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_ENVELOPEQUERYTIMEPARAMESTIMATOR_HPP
