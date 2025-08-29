#include "Index/Estimator/EnvelopeConfigGenerator/EnvelopeConfigGenerator.hpp"

#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/RunSettings/RunSettings.hpp"

std::pair<size_t, EnvelopeParams> IEnvelopeConfigGenerator::get_envelope_params_and_size(
    const EnvelopeIndexProperties *env_index_params, SaxSegIndT num_segments, Real l_per_group_ratio,
    IndexType index_type, Real index_size_limit) const {
    auto &RS = RunSettings::get_instance();
    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;
    uint l_per_group = U(std::ceil(R(l_max - l_min + 1) * l_per_group_ratio));

    LengthProperties length_props{.m_l_min = l_min, .m_l_max = l_max};
    length_props.set_lengths_per_group(l_per_group);

    auto &dataset_props = RS.get_dataset_props();
    uptr<ILengthGroupSegmentationStrategy> lg_segmentation_strategy = nullptr;
    if (env_index_params) {
        auto index_params = std::make_unique<EnvelopeIndexProperties>(*env_index_params);
        index_params->m_segmentation_params.m_num_segments = num_segments;
        GeneralIndexProperties act_index_opts{
            .m_use_length_groups = true,
            .m_num_channels = dataset_props.m_num_channels,
            .m_index_method = index_type,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_series_len = dataset_props.m_series_len,
            .m_l_per_group = l_per_group,
            .m_index_params = std::move(index_params),
        };
        lg_segmentation_strategy = get_lg_segmentation_strategy(act_index_opts);
    }
    IndexSizeEstimator size_estimator(index_type, length_props, lg_segmentation_strategy.get(), num_segments);
    uint pos_per_env = size_estimator.get_min_pos_per_env(index_size_limit);
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
