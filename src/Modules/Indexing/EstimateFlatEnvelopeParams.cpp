#include "Modules/Indexing/EstimateFlatEnvelopeParams.hpp"

#include "Enums/FlatEnvelopeParamEstimatorType.hpp"
#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"
#include "Index/Estimator/FlatEnvelopeTheoParamEstimator.hpp"
#include "Index/Estimator/Sampling/FlatEnvelopeMinDistParamEstimator.hpp"
#include "Index/IndexOptions.hpp"
#include "Util/RunSettings/RunSettings.hpp"

std::optional<FlatEnvelopeParams> estimate_flat_envelope_params(const IndexOptions &opts) {
    auto estimator_params = opts.m_estimator_params.get();
    if (estimator_params) {
        uptr<IFlatEnvelopeParamEstimator> estimator;
        switch (estimator_params->m_param_estimator_type) {
            case THEORETICAL:
                estimator = std::make_unique<FlatEnvelopeParamTheoEstimator>(opts);
                break;
            case MIN_DIST:
                estimator = std::make_unique<FlatEnvelopeMinDistParamEstimator>(opts);
                break;
            case QUERY_TIME:
                throw std::runtime_error("Not yet implemented");
        }
        return estimator->get_estimated_params();
    }
    return std::nullopt;
}
