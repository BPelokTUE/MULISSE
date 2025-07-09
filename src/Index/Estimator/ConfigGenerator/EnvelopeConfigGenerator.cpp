#include "Index/Estimator/EnvelopeConfigGenerator/EnvelopeConfigGenerator.hpp"

#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/RunSettings/RunSettings.hpp"

std::pair<size_t, EnvelopeParams> IEnvelopeConfigGenerator::get_envelope_params_and_size(SaxSegIndT num_segments,
                                                                                         Real l_per_group_ratio,
                                                                                         SearchMethodType index_type,
                                                                                         Real index_size_limit) const {
    auto &RS = RunSettings::get_instance();
    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;
    uint l_per_group = U(std::ceil(R(l_max - l_min + 1) * l_per_group_ratio));

    LengthProperties length_props{.m_l_min = l_min, .m_l_max = l_max};
    length_props.set_lengths_per_group(l_per_group);

    IndexSizeEstimator size_estimator(index_type, length_props, nullptr, num_segments);
    uint pos_per_env = size_estimator.get_max_pos_per_env(index_size_limit);
    size_t estimated_size = size_estimator.get_estimated_flat_envelope_size(pos_per_env);
    return {estimated_size, EnvelopeParams{
                                .m_num_segments = num_segments,
                                .m_pos_per_env = pos_per_env,
                                .m_l_per_group = l_per_group,
                            }};
}

size_t IEnvelopeConfigGenerator::get_bytes_limit(Real index_size_limit) const {
    return static_cast<size_t>(index_size_limit * R(get_dataset_size(RunSettings::get_instance().get_dataset_path())));
}
