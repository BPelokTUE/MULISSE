#include "Index/Estimator/EnvelopeOnlyGammaEstimator.hpp"

EnvelopeParams EnvelopeOnlyGammaEstimator::get_estimated_params(const GeneralIndexProperties &index_opts,
                                                                uptr<IEnvelopeConfigGenerator> env_config_generator) {
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
                .m_pos_per_env = index_size_estimator.get_min_pos_per_env(estimator_params->m_index_size_limit),
                .m_l_per_group = opts.m_l_per_group,
            };
        }
    }
}
