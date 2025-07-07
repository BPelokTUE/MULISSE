#ifndef INDEX_ESTIMATOR_SAMPLING_ENVELOPEQUERYTIMEPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_SAMPLING_ENVELOPEQUERYTIMEPARAMESTIMATOR_HPP

#include "Index/Estimator/Sampling/EnvelopeSamplingParamEstimator.hpp"
#include "Index/Estimator/Sampling/EstimatorSamplingParams.hpp"

class EnvelopeQueryTimeParamEstimator : public EnvelopeSamplingParamEstimator {
   public:
    /**
     * @brief Constructor for EnvelopeParamQueryTimeEstimator
     * @param index_opts The initial index options to use for the FlatEnvelopeIndex
     */
    EnvelopeQueryTimeParamEstimator(const IndexOptions &index_opts);

   protected:
    void update_queries(std::stringstream &query_stream, uint num_queries) override;

    Real get_config_score(vec<vec<IndexEntry<Envelope>>> &&entries, const IndexOptions &config_opts,
                          const LengthProperties &length_props,
                          const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) override;

   private:
    EnvelopeParams m_estimated_params;
    vec<vec<vec<Real>>> m_queries;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_ENVELOPEQUERYTIMEPARAMESTIMATOR_HPP
