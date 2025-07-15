#include "Index/Estimator/EnvelopeConfigGenerator/RandomEnvConfigGenerator.hpp"

#include <random>

#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Util/RunSettings/RunSettings.hpp"

RandomEnvConfigGenerator::RandomEnvConfigGenerator(RandomEnvConfigGeneratorParams params) : m_params(params) {}

vec<EnvelopeParams> RandomEnvConfigGenerator::generate_configurations(const EnvelopeIndexParams *env_index_params,
                                                                      SearchMethodType index_type,
                                                                      Real index_size_limit) {
    auto &RS = RunSettings::get_instance();
    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;

    size_t size_limit_bytes = get_bytes_limit(index_size_limit);
    vec<EnvelopeParams> configurations(m_params.m_num_configs);

    std::uniform_int_distribution<SaxSegIndT> num_segments_dist(m_params.m_num_segments_min,
                                                                m_params.m_num_segments_max);
    std::uniform_real_distribution<Real> num_len_groups_dist(1.0, R(l_max - l_min + 1));
    std::mt19937 rng(m_params.m_seed);

    uint configs_generated = 0;
    while (configs_generated < m_params.m_num_configs) {
        SaxSegIndT num_segments = num_segments_dist(rng);
        Real num_len_groups = num_len_groups_dist(rng);
        Real l_per_group_ratio = 1.0 / num_len_groups;
        auto [estimated_size, env_params] = get_envelope_params_and_size(
            env_index_params, num_segments, l_per_group_ratio, index_type, index_size_limit);

        if (estimated_size > 0 && estimated_size <= size_limit_bytes) {
            configurations[configs_generated++] = env_params;
        }
    }
    return configurations;
}
