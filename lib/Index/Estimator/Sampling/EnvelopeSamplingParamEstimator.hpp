#ifndef INDEX_ESTIMATOR_SAMPLING_ENVELOPESAMPLINGPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_SAMPLING_ENVELOPESAMPLINGPARAMESTIMATOR_HPP

#include <sstream>

#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Estimator/EnvelopeParamEstimator.hpp"
#include "Util/Types/Vec.hpp"

class Envelope;

template <typename T>
class IndexEntry;

class LengthProperties;

class ILengthGroupSegmentationStrategy;

class EnvelopeSamplingParamEstimator : public IEnvelopeParamEstimator {
   public:
    virtual ~EnvelopeSamplingParamEstimator() = default;

    EnvelopeParams get_estimated_params(const GeneralIndexProperties &index_opts,
                                        uptr<IEnvelopeConfigGenerator> env_config_generator) override;

   protected:
    virtual void update_queries(std::stringstream &query_stream, uint num_queries) = 0;

    virtual Real get_config_score(vec<vec<IndexEntry<Envelope>>> &&entries, const GeneralIndexProperties &index_opts,
                                  const LengthProperties &length_props,
                                  const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) = 0;

    vec<uint> m_mts_inds;

   private:
    EnvelopeParams m_estimated_params;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_ENVELOPESAMPLINGPARAMESTIMATOR_HPP
