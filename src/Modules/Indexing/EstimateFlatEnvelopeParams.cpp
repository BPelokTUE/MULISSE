#include "Modules/Indexing/EstimateFlatEnvelopeParams.hpp"

#include "Enums/FlatEnvelopeParamEstimatorType.hpp"
#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"
#include "Index/Estimator/FlatEnvelopeTheoParamEstimator.hpp"
#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Index/Estimator/Sampling/FlatEnvelopeMinDistParamEstimator.hpp"
#include "Index/Estimator/Sampling/FlatEnvelopeQueryTimeParamEstimator.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"
#include "Util/Logging/ParamEstimatesLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

std::optional<FlatEnvelopeParams> estimate_flat_envelope_params(IndexOptions &opts) {
    // If l_per_group and num_segments are set, use the maximum pos_per_env value given the index size limit.
    if (auto paa_index_params = dynamic_cast<PaaIndexParams *>(opts.m_index_params.get())) {
        SaxSegIndT num_segments = paa_index_params->m_segmentation_params.m_num_segments;
        if (num_segments > 0 && opts.m_l_per_group > 0) {
            auto tmp_lgss = get_lg_segmentation_strategy(opts);
            IndexSizeEstimator index_size_estimator{
                opts.m_index_method,
                RunSettings::get_instance().get_length_props(),
                tmp_lgss.get(),
                num_segments,
            };
            return FlatEnvelopeParams{
                .m_num_segments = num_segments,
                .m_pos_per_env = index_size_estimator.get_max_pos_per_env(opts.m_estimator_params->m_index_size_limit),
                .m_l_per_group = opts.m_l_per_group,
            };
        }
    }

    // Otherwise, estimate the parameters using the FlatEnvelopeParamEstimator.
    auto estimator_params = opts.m_estimator_params.get();
    if (estimator_params) {
        ParamEstimatesLogger::initialize();

        uptr<IFlatEnvelopeParamEstimator> estimator;
        switch (estimator_params->m_param_estimator_type) {
            case THEORETICAL:
                estimator = std::make_unique<FlatEnvelopeParamTheoEstimator>(opts);
                break;
            case MIN_DIST:
                estimator = std::make_unique<FlatEnvelopeMinDistParamEstimator>(opts);
                break;
            case QUERY_TIME:
                estimator = std::make_unique<FlatEnvelopeQueryTimeParamEstimator>(opts);
                break;
        }
        return estimator->get_estimated_params();
    }
    return std::nullopt;
}
