#include "Index/Estimator/ConfigGenerator/DummyConfigGenerator.hpp"

#include <cmath>

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/FlatEnvelopeSizeEstimator.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/RunSettings/RunSettings.hpp"

vec<FlatEnvelopeParams> DummyConfigGenerator::generate_configurations(Real index_size_limit) {
    auto &RS = RunSettings::get_instance();
    size_t size_limit_bytes = static_cast<size_t>(index_size_limit * R(get_dataset_size(RS.get_dataset_path())));

    vec<SaxSegIndT> num_segments_vals = {32, 16, 8, 4};
    vec<Real> lg_size_ratio_vals = {R(0.05), R(0.2), R(0.5), R(1.0)};

    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;
    uint l_range = l_max - l_min + 1;

    vec<FlatEnvelopeParams> configurations;
    for (SaxSegIndT num_segments : num_segments_vals)
        for (Real lg_size_ratio : lg_size_ratio_vals) {
            uint l_per_group = U(std::ceil(R(l_range) * lg_size_ratio));
            uint num_l_groups = (l_range + l_per_group - 1) / l_per_group;
            LengthProperties length_props{
                .m_use_length_groups = true,
                .m_l_min = l_min,
                .m_l_max = l_max,
                .m_l_per_group = l_per_group,
                .m_num_l_groups = num_l_groups,
            };

            FlatEnvelopeSizeEstimator size_estimator(length_props, nullptr, num_segments);
            uint pos_per_env = size_estimator.get_max_pos_per_env(index_size_limit);
            size_t estimated_size = size_estimator.get_estimated_flat_envelope_size(pos_per_env);
            if (size_limit_bytes >= estimated_size) {
                configurations.push_back(FlatEnvelopeParams{
                    .m_num_segments = num_segments,
                    .m_pos_per_env = pos_per_env,
                    .m_l_per_group = l_per_group,
                });
            }
        }

    return configurations;
}
