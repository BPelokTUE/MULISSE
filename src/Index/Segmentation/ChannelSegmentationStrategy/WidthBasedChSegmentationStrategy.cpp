#include "Index/Segmentation/ChannelSegmentationStrategy/WidthBasedChSegmentationStrategy.hpp"

#include <algorithm>
#include <random>

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToSegmentationStrategy.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Stats/EnvelopeStatsUtil.hpp"

WidthBasedChSegmentationStrategy::WidthBasedChSegmentationStrategy(
    const IScoreToSegmentationStrategy *score_to_segmentation_strategy, SamplingParams sampling_params,
    Real min_mean_update)
    : m_score_to_segmentation_strategy(score_to_segmentation_strategy), m_min_mean_update(min_mean_update) {
    initialize(sampling_params);
}

void WidthBasedChSegmentationStrategy::initialize_segmentation_strategies() {
    vec<Real> range_sums;
    Real range_min = INF, range_max = 0.0;
    uint sample_count = 0;

    volatile bool early_stop = false;

    OMP_PRAGMA(omp parallel) {
        std::ifstream dataset_ifs(*m_late_init_params->m_dataset_path, std::ios::binary);
        OMP_PRAGMA(omp for)
        for (uint i = 0; i < m_late_init_params->m_sample_size; ++i) {
            if (early_stop) continue;

            vec<vec<Real>> mts(m_late_init_params->m_num_channels, vec<Real>(m_late_init_params->m_series_len));
            for (MtsNumChannelsT c = 0; c < m_late_init_params->m_num_channels; ++c) {
                uint mts_ind = m_late_init_params->m_mts_inds[c][i];
                dataset_ifs.seekg(mts_ind * m_late_init_params->m_series_len * m_late_init_params->m_num_channels *
                                  sizeof(Real));
                dataset_ifs.read(reinterpret_cast<char *>(mts[c].data()),
                                 m_late_init_params->m_series_len * sizeof(Real));
            }
            auto envelope = m_late_init_params->m_generator->get_entries(mts, 0)[0][0].m_mts_summary;
            OMP_PRAGMA(omp critical) {
                ++sample_count;
                bool sufficient_update = false;
                for (MtsNumChannelsT c = 0; c < m_late_init_params->m_num_channels; ++c) {
                    auto &summary = envelope[c];
                    for (SaxSegIndT s = 0; s < summary.size(); ++s) {
                        Real lower = summary.m_lower[s], upper = summary.m_upper[s];
                        Real range = upper - lower;

                        if (range < range_min) range_min = range;
                        if (range > range_max) range_max = range;
                        Real prev_range_mean = sample_count > 1 ? range_sums[c] / R(sample_count - 1) : 0.0;
                        range_sums[c] += range;
                        Real range_update = range_sums[c] / R(sample_count) - prev_range_mean;
                        sufficient_update |= std::abs(range_update) / (range_max - range_min) >= m_min_mean_update;
                    }
                }
                if (!sufficient_update) early_stop = true;
            }
        }
    }

    vec<Real> scores;
    scores.reserve(m_late_init_params->m_num_channels);
    for (Real range_sum : range_sums) {
        scores.push_back((range_sum / R(sample_count) - range_min) / (range_max - range_min));
    }

    m_segmentation_strategies = m_score_to_segmentation_strategy->get_segmentation_strategies(scores);
}

ChannelSegmentationStrategyType WidthBasedChSegmentationStrategy::get_type() const {
    return ChannelSegmentationStrategyType::WIDTH_BASED;
}
