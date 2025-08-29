#include "Util/Artefacts/IndexManager.hpp"

#include "Index/Estimator/EnvelopeConfigGenerator/GridEnvConfigGenerator.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/RandomEnvConfigGenerator.hpp"
#include "Index/Estimator/EnvelopeOnlyGammaEstimator.hpp"
#include "Index/Estimator/EnvelopeTheoParamEstimator.hpp"
#include "Index/Estimator/Sampling/EnvelopeMinDistParamEstimator.hpp"
#include "Index/Estimator/Sampling/EnvelopeQueryTimeParamEstimator.hpp"
#include "Index/Sax/BreakpointStrategy/EquiprobableBreakpointStrategy.hpp"
#include "Index/Sax/BreakpointStrategy/FixedBreakpointStrategy.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/Options/IndexGenOptions.hpp"
#include "Util/Artefacts/Properties/GeneralIndexProperties.hpp"
#include "Util/Artefacts/Properties/SpecificIndexProperties.hpp"
#include "Util/HelperFuncs/Containers.hpp"
#include "Util/HelperFuncs/Errors.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/Logging/ParamEstimatesLogger.hpp"

void IndexManager::initialize_sax_breakpoints(const SaxProperties &sax_props) {
    m_sax_breakpoint_strategy = nullptr;
    switch (sax_props.m_breakpoint_strategy_type) {
        case EQUIPROBABLE:
            m_sax_breakpoint_strategy = std::make_unique<EquiprobableBreakpointStrategy>();
        case FIXED:
            m_sax_breakpoint_strategy = std::make_unique<FixedBreakpointStrategy>(sax_props.m_breakpoints_file);
    }
    m_sax_breakpoints = m_sax_breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << sax_props.m_num_bits));
}

void IndexManager::initialize_sax_breakpoints() {
    auto *paa_index_props = dynamic_cast<const PaaIndexProperties *>(&m_specific_index_props);
    if (arr_contains(METHODS_W_ISAX, m_general_index_props.m_index_method)) {
        auto *index_props = dynamic_cast<const iSaxIndexProperties *>(&m_specific_index_props);
        if (index_props && index_props->m_sax_params.m_num_bits > 0) {
            initialize_sax_breakpoints(index_props->m_sax_params);
        } else {
            throw std::runtime_error("iSAX index method requires a positive number of bits");
        }
    } else if (arr_contains(METHODS_W_SAX, m_general_index_props.m_index_method)) {
        auto *index_props = dynamic_cast<const SaxIndexProperties *>(&m_specific_index_props);
        if (index_props && index_props->m_sax_params.m_num_bits > 0) {
            initialize_sax_breakpoints(index_props->m_sax_params);
        } else {
            throw std::runtime_error("SAX index method requires a positive number of bits");
        }
    } else if (arr_contains(METHODS_W_PAA, m_general_index_props.m_index_method)) {
        if (paa_index_props && arr_contains(MERGERS_W_SAX, paa_index_props->m_merger_params.m_entry_merger_type)) {
            auto merger_sax_props = paa_index_props->m_merger_params.m_merger_sax_props;
            if (merger_sax_props && merger_sax_props->m_num_bits > 0) {
                initialize_sax_breakpoints(*merger_sax_props);
            } else {
                throw std::runtime_error("SAX-based envelope entry merger requires a positive number of bits");
            }
        }
    }
}

EnvelopeIndexProperties &IndexManager::get_envelope_index_props() {
    auto *env_index_props = dynamic_cast<EnvelopeIndexProperties *>(&m_specific_index_props);
    if (!env_index_props) throw std::runtime_error("Envelope index methods require EnvelopeIndexParams.");
    return *env_index_props;
}

void IndexManager::set_optimal_params() {
    if (!m_source_dataset) throw errors::source_dataset_not_set();

    uint series_len = m_source_dataset->get().get_properties().m_series_len;
    auto [l_min, l_max] = m_general_index_props.m_l_range;
    uint num_ls = l_max - l_min + 1;

    if (arr_contains(FLAT_ENVELOPE_METHODS, m_general_index_props.m_index_method)) {
        auto &env_index_params = get_envelope_index_props();
        EnvelopeParams envelope_params;

        if (m_normalized) {
            Real num_seg_multiplier = env_index_params.m_segmentation_params.m_strategy_type == UNIFORM ? 16.0 : 12.0;
            uint optimal_num_l_groups = 40;

            envelope_params.m_num_segments =
                static_cast<SaxSegIndT>(std::ceil(std::sqrt(R(series_len) / R(2 * l_min)) * num_seg_multiplier));
            envelope_params.m_pos_per_env = 64;
            envelope_params.m_l_per_group = (num_ls + optimal_num_l_groups - 1) / optimal_num_l_groups;
        } else {
            envelope_params.m_num_segments = static_cast<SaxSegIndT>(std::ceil(R(2 * series_len) / R(l_min)));
            envelope_params.m_pos_per_env = 256;
            envelope_params.m_l_per_group = num_ls;
        }
        env_index_params.set_flat_envelope_params(envelope_params);
    }
}

void IndexManager::estimate_params(const IndexGenOptions &gen_opts) {
    if (auto *estimator_params = gen_opts.m_estimator_params.get(); estimator_params) {
        uptr<IEnvelopeConfigGenerator> env_config_generator = nullptr;
        switch (estimator_params->m_config_generator_type) {
            case RANDOM:
                if (auto random_params = dynamic_cast<RandomEnvConfigGeneratorParams *>(
                        estimator_params->m_config_generator_params.get())) {
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
                if (estimator_params->m_qt_distance_type == ED) {
                    if (estimator_params->m_qt_examine_whole) {
                        estimator = std::make_unique<EnvelopeQueryTimeParamEstimator<ED, true>>();
                    } else {
                        estimator = std::make_unique<EnvelopeQueryTimeParamEstimator<ED, false>>();
                    }
                } else {  // D == MASS
                    if (estimator_params->m_qt_examine_whole) {
                        estimator = std::make_unique<EnvelopeQueryTimeParamEstimator<MASS, true>>();
                    } else {
                        estimator = std::make_unique<EnvelopeQueryTimeParamEstimator<MASS, false>>();
                    }
                }
                break;
                // case ONLY_GAMMA: {
                //     auto &envelope_index_props = get_envelope_index_props();
                //     EnvelopeParams envelope_params = {
                //         .m_num_segments = envelope_index_props.m_segmentation_params.m_num_segments,
                //         .m_pos_per_env = envelope_index_props.m_pos_per_env,
                //         .m_l_per_group = m_general_index_props.m_l_per_group,
                //     };
                //     estimator = std::make_unique<EnvelopeOnlyGammaEstimator>(envelope_params);
                //     break;
        }
        auto envelope_params = estimator->get_estimated_params(m_general_index_props, std::move(env_config_generator));

        auto &envelope_index_props = get_envelope_index_props();
        envelope_index_props.set_flat_envelope_params(envelope_params);
        m_general_index_props.m_l_per_group = envelope_params.m_l_per_group;
    }
}
