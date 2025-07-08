#include "Index/Estimator/EnvelopeConfigGenerator/GridEnvConfigGenerator.hpp"

#include <cmath>

#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

vec<EnvelopeParams> GridEnvConfigGenerator::generate_configurations(SearchMethodType index_type,
                                                                    Real index_size_limit) {
    size_t size_limit_bytes = get_bytes_limit(index_size_limit);

    vec<SaxSegIndT> num_segments_vals = {32, 16, 8, 4};
    vec<Real> l_per_group_ratios = {R(0.05), R(0.2), R(0.5), R(1.0)};

    vec<EnvelopeParams> configurations;
    for (Real l_per_group_ratio : l_per_group_ratios) {
        for (SaxSegIndT num_segments : num_segments_vals) {
            auto [estimated_size, env_params] =
                get_envelope_params_and_size(num_segments, l_per_group_ratio, index_type, index_size_limit);

            if (estimated_size > 0 && estimated_size <= size_limit_bytes) configurations.push_back(env_params);
        }
    }
    return configurations;
}
