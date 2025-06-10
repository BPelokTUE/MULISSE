#include "Index/Estimator/ConfigGenerator/DummyConfigGenerator.hpp"

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"

vec<FlatEnvelopeParams> DummyConfigGenerator::generate_configurations(Real index_size_limit) {
    auto &RS = RunSettings::get_instance();

    vec<SaxSegIndT> num_segments_vals = {32, 16, 8, 4};
    vec<Real> lg_size_ratio_vals = {R(0.05), R(0.2), R(0.5), R(1.0)};

    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;

    vec<FlatEnvelopeParams> configurations;
    configurations.reserve(num_segments_vals.size() * lg_size_ratio_vals.size());
    for (SaxSegIndT num_segments : num_segments_vals)
        for (Real lg_size_ratio : lg_size_ratio_vals)
            configurations.push_back(FlatEnvelopeParams{
                .m_num_segments = num_segments,
                .m_pos_per_env = get_max_pos_per_env(index_size_limit, nullptr, num_segments),
                .m_l_per_group = U(R(l_max - l_min + 1) * lg_size_ratio),
            });

    return configurations;
}
