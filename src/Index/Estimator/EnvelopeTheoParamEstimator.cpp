#include "Index/Estimator/EnvelopeTheoParamEstimator.hpp"

#include <cmath>

#include "Index/Entry/Envelope.hpp"
#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/GridEnvConfigGenerator.hpp"
#include "Index/IndexOptions.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Logging/ParamEstimatesLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

using std::pow;

EnvelopeParams EnvelopeParamTheoEstimator::get_estimated_params(const IndexOptions &index_opts,
                                                                const IEnvelopeConfigGenerator *env_config_generator) {
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

    uint seed = 211;                                                  // TMP
    uint x_p = 1000;                                                  // TMP
    uint x_s = 100;                                                   // TMP
    std::normal_distribution<Real> query_noise_dist(R(0.0), R(0.1));  // TMP

    auto &RS = RunSettings::get_instance();
    auto &logger = ParamEstimatesLogger::get_instance();

    vec<EnvelopeParams> configurations = GridEnvConfigGenerator().generate_configurations(
        index_opts.m_index_method, index_opts.m_estimator_params->m_index_size_limit);

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
            SaxSegIndT num_segments = static_cast<SaxSegIndT>(length_props.get_lg_l_max(lg_ind) / segment_len);
            std::student_t_distribution<Real> student_t_dist(R(lengths[j] - 1));

            for (SaxSegIndT seg_ind = 0; seg_ind < num_segments; ++seg_ind) {
                PaaDistributionInputs paa_dist_inputs{seg_ind, lengths[j], start_positions[j], segment_len};
                Real paa_stdev = get_paa_stdev(paa_dist_inputs);
                for (uint paa_sample_ind = 0; paa_sample_ind < x_s; ++paa_sample_ind) {
                    Real paa_segment = paa_stdev * student_t_dist(rng);
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

        logger.write_entry(config, min_distance_sums[i]);
    }
    return configurations[selected_config_index];
}

Real EnvelopeParamTheoEstimator::get_paa_stdev(const PaaDistributionInputs &inputs) {
    Real k = R(inputs.seg_ind) + 1, l = R(inputs.length), p = R(inputs.start_pos) + 1, s = R(inputs.segment_len);
    Real paa_var =
        R(l * pow(4 * pow(l, 2) + 12 * l * p - 6 * l + 12 * pow(p, 2) - 12 * p - 3 * pow(l + 2 * p - 1, 2) + 2, 2) *
              (6 * pow(k, 2) * pow(s, 3) + 12 * k * p * pow(s, 2) - 6 * k * pow(s, 3) - 6 * k * pow(s, 2) +
               2 * pow(l, 2) + 6 * l * p + l * s * (6 * k * pow(s, 2) + 6 * p * s - 4 * pow(s, 2) - 3 * s + 1) - 3 * l +
               6 * pow(p, 2) * s - 6 * p * pow(s, 2) + 6 * p * s * (p - 1) - 6 * p * s + 2 * pow(s, 3) + 3 * pow(s, 2) -
               3 * s * (2 * l + 2 * p - 1) * (2 * k * s + 2 * p - s - 1) + s + 1) +
          R(1.0 / 5.0) * l *
              R(4 * pow(l, 2) + 12 * l * p - 6 * l + 12 * pow(p, 2) - 12 * p - 3 * pow(l + 2 * p - 1, 2) + 2) *
              R(90 * pow(k, 4) * pow(l, 2) * pow(s, 4) - 180 * pow(k, 3) * pow(l, 3) * pow(s, 3) -
                360 * pow(k, 3) * pow(l, 2) * pow(s, 4) + 120 * pow(k, 2) * pow(l, 4) * pow(s, 2) +
                360 * pow(k, 2) * pow(l, 3) * pow(s, 3) + 150 * pow(k, 2) * pow(l, 3) * pow(s, 2) -
                360 * pow(k, 2) * pow(l, 2) * p * pow(s, 3) + 540 * pow(k, 2) * pow(l, 2) * p * pow(s, 2) +
                540 * pow(k, 2) * pow(l, 2) * pow(s, 4) + 180 * pow(k, 2) * pow(l, 2) * pow(s, 3) -
                300 * pow(k, 2) * pow(l, 2) * pow(s, 2) + 120 * pow(k, 2) * l * pow(s, 2) - 30 * k * pow(l, 5) * s -
                120 * k * pow(l, 4) * pow(s, 2) - 120 * k * pow(l, 4) * s + 360 * k * pow(l, 3) * p * pow(s, 2) -
                540 * k * pow(l, 3) * p * s - 150 * k * pow(l, 3) * pow(s, 3) - 480 * k * pow(l, 3) * pow(s, 2) +
                300 * k * pow(l, 3) * s + 900 * k * pow(l, 2) * p * pow(s, 3) - 1080 * k * pow(l, 2) * p * pow(s, 2) -
                360 * k * pow(l, 2) * pow(s, 4) - 450 * k * pow(l, 2) * pow(s, 3) + 660 * k * pow(l, 2) * pow(s, 2) -
                150 * k * pow(l, 2) * s + 60 * k * l * pow(s, 3) - 240 * k * l * pow(s, 2) + 2 * pow(l, 6) +
                36 * pow(l, 5) - 120 * pow(l, 4) * p * s + 180 * pow(l, 4) * p + 60 * pow(l, 4) * pow(s, 2) +
                60 * pow(l, 4) * s - 58 * pow(l, 4) + 30 * pow(l, 3) * p * pow(s, 2) - 300 * pow(l, 3) * p * s +
                330 * pow(l, 3) * p - 20 * pow(l, 3) * pow(s, 3) + 135 * pow(l, 3) * pow(s, 2) + 155 * pow(l, 3) * s -
                165 * pow(l, 3) + 540 * pow(l, 2) * pow(p, 2) * pow(s, 2) - 1080 * pow(l, 2) * pow(p, 2) * s +
                540 * pow(l, 2) * pow(p, 2) - 480 * pow(l, 2) * p * pow(s, 3) + 1230 * pow(l, 2) * p * s -
                720 * pow(l, 2) * p + 90 * pow(l, 2) * pow(s, 4) + 240 * pow(l, 2) * pow(s, 3) -
                195 * pow(l, 2) * pow(s, 2) - 345 * pow(l, 2) * s + 308 * pow(l, 2) + 60 * l * p * pow(s, 2) -
                240 * l * p * s + 210 * l * p - 40 * l * pow(s, 3) + 90 * l * pow(s, 2) + 130 * l * s - 141 * l + 18) +
          R(1.0 / 2.0) *
              R(120 * pow(l, 6) + 1080 * pow(l, 5) * p - 356 * pow(l, 5) + 3240 * pow(l, 4) * pow(p, 2) -
                2208 * pow(l, 4) * p + 474 * pow(l, 4) + 3240 * pow(l, 3) * pow(p, 3) - 3348 * pow(l, 3) * pow(p, 2) +
                1998 * pow(l, 3) * p - 407 * pow(l, 3) + 1620 * pow(l, 2) * pow(p, 2) - 1140 * pow(l, 2) * p +
                255 * pow(l, 2) + 270 * l * p - 101 * l + 15) *
              R(6 * pow(k, 2) * pow(s, 3) + 12 * k * p * pow(s, 2) - 6 * k * pow(s, 3) - 6 * k * pow(s, 2) +
                2 * pow(l, 2) + 6 * l * p + l * s * (6 * k * pow(s, 2) + 6 * p * s - 4 * pow(s, 2) - 3 * s + 1) -
                3 * l + 6 * pow(p, 2) * s - 6 * p * pow(s, 2) + 6 * p * s * (p - 1) - 6 * p * s + 2 * pow(s, 3) +
                3 * pow(s, 2) - 3 * s * (2 * l + 2 * p - 1) * (2 * k * s + 2 * p - s - 1) + s + 1)) /
        R(l * pow(s, 2) *
          pow(4 * pow(l, 2) + 12 * l * p - 6 * l + 12 * pow(p, 2) - 12 * p - 3 * pow(l + 2 * p - 1, 2) + 2, 3));

    return std::sqrt(std::max(paa_var, EPS));
}
