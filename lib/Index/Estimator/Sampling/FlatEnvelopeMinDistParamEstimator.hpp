#ifndef INDEX_ESTIMATOR_SAMPLING_FLATENVELOPEMINDISTPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_SAMPLING_FLATENVELOPEMINDISTPARAMESTIMATOR_HPP

#include "Index/Estimator/Sampling/EstimatorSamplingParams.hpp"
#include "Index/Estimator/Sampling/FlatEnvelopeSamplingParamEstimator.hpp"

class FlatEnvelopeMinDistParamEstimator : public FlatEnvelopeSamplingParamEstimator {
   public:
    /**
     * @brief Constructor for FlatEnvelopeParamTheoEstimator
     * @param opts The initial index options to use for the FlatEnvelopeIndex
     */
    FlatEnvelopeMinDistParamEstimator(const IndexOptions &opts);

   protected:
    void update_queries(std::stringstream &query_stream, uint num_queries) override;

    Real get_config_score(const vec<vec<IndexEntry<Envelope>>> entries, const IndexOptions &index_opts,
                          const LengthProperties &length_props,
                          const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) override;

   private:
    FlatEnvelopeParams m_estimated_params;
    vec<vec<vec<Real>>> m_query_accs;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_FLATENVELOPEMINDISTPARAMESTIMATOR_HPP
