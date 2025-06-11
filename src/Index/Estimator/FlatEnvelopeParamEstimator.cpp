#include "Index/Estimator/FlatEnvelopeParamEstimator.hpp"

#include <cmath>

#include "Index/Entry/Envelope.hpp"
#include "Index/Estimator/ConfigGenerator/DummyConfigGenerator.hpp"
#include "Index/IndexOptions.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

FlatEnvelopeParamEstimator::FlatEnvelopeParamEstimator(const IndexOptions &opts) {
    // Generate X_c configurations
    // Sample X_p l-p pairs
    // For each configuration
    //     Initialize N_l*N_p empty envelopes with N_s^i segments each
    //     Initialize accumulators for the min-distance
    //     For each l-p pair
    //         Get the distributions of all k segments
    //         Update the segments of the relevant envelope with X_s samples
    //         Update the min-distance accumulators using X_q sampled query PAAs
    // Select the configuration with the biggest average min-distance

    uint seed = 100;                                                  // TMP
    uint x_p = 1000;                                                  // TMP
    uint x_s = 100;                                                   // TMP
    std::normal_distribution<Real> query_noise_dist(R(0.0), R(0.1));  // TMP

    auto &RS = RunSettings::get_instance();

    vec<FlatEnvelopeParams> configurations =
        DummyConfigGenerator().generate_configurations(opts.m_estimator_params->m_index_size_limit);

    std::default_random_engine rng(seed);
    LengthProperties length_props = RS.get_length_props();
    uint l_min = length_props.m_l_min, l_max = length_props.m_l_max;
    uint series_len = RS.get_dataset_props().m_series_len;

    std::uniform_int_distribution<uint> length_dist(l_min, l_max);
    std::uniform_int_distribution<uint> start_pos_dist(0, series_len - l_min);

    vec<uint> lengths(x_p), start_positions(x_p);
    for (uint i = 0; i < x_p; ++i) {
        lengths[i] = length_dist(rng);
        start_positions[i] = start_pos_dist(rng) % (series_len - lengths[i] + 1);
    }

    DistanceMeasure<KNN, ED> distance_measure(true);
    vec<vec<vec<Envelope>>> envelopes(configurations.size());
    vec<Real> min_distance_sums(configurations.size(), R(0.0));

    Real max_min_distance_sum = 0.0;
    uint selected_config_index = 0;

    for (uint i = 0; i < configurations.size(); ++i) {
        const auto &config = configurations[i];

        uint num_lg = (l_max - l_min + config.m_l_per_group) / config.m_l_per_group,
             segment_len = l_max / config.m_num_segments;

        length_props.m_use_length_groups = true;
        length_props.m_num_l_groups = num_lg;
        length_props.m_l_per_group = config.m_l_per_group;

        envelopes[i].resize(num_lg);
        for (uint lg_ind = 0; lg_ind < num_lg; ++lg_ind) {
            uint lg_l_max = length_props.get_lg_l_max(lg_ind), lg_l_min = length_props.get_lg_l_min(lg_ind);
            uint num_env = (series_len - lg_l_min + config.m_pos_per_env) / config.m_pos_per_env;
            envelopes[i][lg_ind].resize(num_env);
            for (uint env_ind = 0; env_ind < num_env; ++env_ind) {
                envelopes[i][lg_ind][env_ind].resize(lg_l_max / segment_len);
            }
        }

        vec<vec<Real>> query_paas(x_p);
        for (uint j = 0; j < x_p; ++j) {
            uint lg_ind = length_props.get_length_group(lengths[j]),
                 env_ind = start_positions[j] / config.m_pos_per_env;
            SaxSegIndT num_segments = length_props.get_lg_l_max(lg_ind) / segment_len;

            for (SaxSegIndT seg_ind = 0; seg_ind < num_segments; ++seg_ind) {
                PaaDistributionInputs paa_dist_inputs{seg_ind, lengths[j], start_positions[j], segment_len};
                auto paa_distribution = get_paa_distribution(paa_dist_inputs);
                for (uint paa_sample_ind = 0; paa_sample_ind < x_s; ++paa_sample_ind) {
                    Real paa_segment = paa_distribution(rng);
                    auto &env_segment = envelopes[i][lg_ind][env_ind];
                    env_segment.m_lower[seg_ind] = std::min(env_segment.m_lower[seg_ind], paa_segment);
                    env_segment.m_upper[seg_ind] = std::max(env_segment.m_upper[seg_ind], paa_segment);

                    if (paa_sample_ind == 0) query_paas[j].push_back(paa_segment + query_noise_dist(rng));
                }
            }
        }

        for (uint j = 0; j < x_p; ++j) {
            uint lg_ind = length_props.get_length_group(lengths[j]);
            auto &query_paa = query_paas[j];

            for (uint env_ind = 0; env_ind < envelopes[i][lg_ind].size(); ++env_ind) {
                for (SaxSegIndT seg_ind = 0; seg_ind < query_paa.size(); ++seg_ind) {
                    min_distance_sums[i] += distance_measure.min_dist_squared(
                        query_paa[seg_ind], envelopes[i][lg_ind][env_ind].m_lower[seg_ind],
                        envelopes[i][lg_ind][env_ind].m_upper[seg_ind]);
                }
            }
        }
        if (min_distance_sums[i] > max_min_distance_sum) {
            max_min_distance_sum = min_distance_sums[i];
            selected_config_index = i;
        }
    }
    m_estimated_params = configurations[selected_config_index];
}

FlatEnvelopeParams FlatEnvelopeParamEstimator::get_estimated_params() { return m_estimated_params; }

std::normal_distribution<Real> FlatEnvelopeParamEstimator::get_paa_distribution(const PaaDistributionInputs &inputs) {
    Real denominator_ev_term = get_denominator_ev_term(inputs);
    Real squared_diff_ev_term = get_squared_diff_ev_term(inputs);
    Real covariance_term = get_covariance_term(inputs);
    Real variance_term = get_variance_term(inputs);

    Real paa_var = squared_diff_ev_term / denominator_ev_term - covariance_term / R(std::pow(denominator_ev_term, 2)) +
                   variance_term / R(std::pow(denominator_ev_term, 3));

    return std::normal_distribution<Real>(R(0.0), std::max(paa_var, EPS));
}

Real FlatEnvelopeParamEstimator::get_denominator_ev_term(const PaaDistributionInputs &inputs) {
    Real l = R(inputs.length), p = R(inputs.start_pos), s = R(inputs.segment_len);
    return R(s * s * (l * l + 6 * p * p - 6 * p - 1) / (6 * l));
}

Real FlatEnvelopeParamEstimator::get_squared_diff_ev_term(const PaaDistributionInputs &inputs) {
    Real k = R(inputs.seg_ind), l = R(inputs.length), p = R(inputs.start_pos), s = R(inputs.segment_len);
    return R((std::pow(s, 3) * (6 * k * k - 6 * k + 2) + 6 * p * p - 6 * p - 2 * s + 1) / (6 * l) +
             (2 * l + 6 * p - 3 * s * s + 3 * s - 3) / 6);
}

Real FlatEnvelopeParamEstimator::get_covariance_term(const PaaDistributionInputs &inputs) { return R(0.0); }  // TMP

Real FlatEnvelopeParamEstimator::get_variance_term(const PaaDistributionInputs &inputs) { return R(0.0); }  // TMP
