#ifndef INDEX_ESTIMATOR_SAMPLING_FLATENVELOPESAMPLINGPARAMESTIMATOR_HPP
#define INDEX_ESTIMATOR_SAMPLING_FLATENVELOPESAMPLINGPARAMESTIMATOR_HPP

#include <sstream>

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"
#include "Util/Types/Containers.hpp"

class Envelope;

template <typename T>
class IndexEntry;

struct IndexOptions;

class LengthProperties;

class ILengthGroupSegmentationStrategy;

class FlatEnvelopeSamplingParamEstimator : public IFlatEnvelopeParamEstimator {
   public:
    virtual ~FlatEnvelopeSamplingParamEstimator() = default;

    /**
     * @brief Constructor for FlatEnvelopeSamplingParamEstimator, checks if the index options contain the required
     * parameters
     * @param index_opts The initial index options to use for the FlatEnvelopeIndex
     */
    FlatEnvelopeSamplingParamEstimator(const IndexOptions &index_opts);

    FlatEnvelopeParams get_estimated_params() override;

   protected:
    /**
     * @brief Estimate the parameters for a FlatEnvelopeIndex based on the provided options
     * @param opts The options to use for estimating the parameters
     */
    void estimate_params(const IndexOptions &opts);

    virtual void update_queries(std::stringstream &query_stream, uint num_queries) = 0;

    virtual Real get_config_score(vec<vec<IndexEntry<Envelope>>> &&entries, const IndexOptions &index_opts,
                                  const LengthProperties &length_props,
                                  const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) = 0;

    vec<uint> m_mts_inds;

   private:
    FlatEnvelopeParams m_estimated_params;
};

#endif  // INDEX_ESTIMATOR_SAMPLING_FLATENVELOPESAMPLINGPARAMESTIMATOR_HPP
