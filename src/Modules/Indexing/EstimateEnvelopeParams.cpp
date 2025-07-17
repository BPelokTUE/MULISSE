#include "Modules/Indexing/EstimateEnvelopeParams.hpp"

#include "Enums/EnvelopeParamEstimatorType.hpp"
#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/EnvelopeConfigGenerator.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/GridEnvConfigGenerator.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/RandomEnvConfigGenerator.hpp"
#include "Index/Estimator/EnvelopeParamEstimator.hpp"
#include "Index/Estimator/EnvelopeTheoParamEstimator.hpp"
#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Index/Estimator/Sampling/EnvelopeMinDistParamEstimator.hpp"
#include "Index/Estimator/Sampling/EnvelopeQueryTimeParamEstimator.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/Logging/ParamEstimatesLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <DistanceType D, bool EW>
std::optional<EnvelopeParams> estimate_envelope_params(IndexOptions &opts) {
    auto estimator_params = opts.m_estimator_params.get();
    if (!estimator_params) return std::nullopt;

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
            return EnvelopeParams{
                .m_num_segments = num_segments,
                .m_pos_per_env = index_size_estimator.get_max_pos_per_env(estimator_params->m_index_size_limit),
                .m_l_per_group = opts.m_l_per_group,
            };
        }
    }

    // Otherwise, estimate the parameters using the EnvelopeParamEstimator.
    ParamEstimatesLogger::initialize();

    auto &index_logger = IndexLogger::get_instance();
    index_logger.start_timer(ISC::ENV_PARAM_ESTIMATION_TIME_S);

    uptr<IEnvelopeConfigGenerator> env_config_generator;
    switch (estimator_params->m_config_generator_type) {
        case RANDOM:
            if (auto random_params =
                    dynamic_cast<RandomEnvConfigGeneratorParams *>(estimator_params->m_config_generator_params.get())) {
                env_config_generator = std::make_unique<RandomEnvConfigGenerator>(*random_params);
            } else {
                throw std::runtime_error("RandomEnvConfigGenerator requires RandomEnvConfigGeneratorParams");
            }
            break;
        case GRID:
            env_config_generator = std::make_unique<GridEnvConfigGenerator>();
            break;
    }

    uptr<IEnvelopeParamEstimator> estimator;
    switch (estimator_params->m_param_estimator_type) {
        case THEORETICAL:
            estimator = std::make_unique<EnvelopeParamTheoEstimator>();
            break;
        case MIN_DIST:
            estimator = std::make_unique<EnvelopeMinDistParamEstimator>();
            break;
        case QUERY_TIME:
            estimator = std::make_unique<EnvelopeQueryTimeParamEstimator<D, EW>>();
            break;
        case NO_EST:
            index_logger.stop_timer(ISC::ENV_PARAM_ESTIMATION_TIME_S);
            return std::nullopt;
    }
    auto estimated_params = estimator->get_estimated_params(opts, std::move(env_config_generator));
    index_logger.stop_timer(ISC::ENV_PARAM_ESTIMATION_TIME_S);
    return estimated_params;
}

// Explicit template specializations
template std::optional<EnvelopeParams> estimate_envelope_params<ED, false>(IndexOptions &opts);
template std::optional<EnvelopeParams> estimate_envelope_params<ED, true>(IndexOptions &opts);
template std::optional<EnvelopeParams> estimate_envelope_params<MASS, false>(IndexOptions &opts);
template std::optional<EnvelopeParams> estimate_envelope_params<MASS, true>(IndexOptions &opts);