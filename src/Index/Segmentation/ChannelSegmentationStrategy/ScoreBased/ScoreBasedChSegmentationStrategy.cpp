#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSegmentationStrategy.hpp"

#include <algorithm>
#include <fstream>
#include <random>

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSSSamplingParams.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScores.hpp"
#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToSegmentationStrategy.hpp"
#include "Util/Types/Pointers.hpp"

ScoreBasedChSegmentationStrategy::~ScoreBasedChSegmentationStrategy() = default;

ScoreBasedChSegmentationStrategy::ScoreBasedChSegmentationStrategy() = default;

ScoreBasedChSegmentationStrategy::ScoreBasedChSegmentationStrategy(
    uptr<IEnvelopeScores> envelope_scores, uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy,
    SamplingChSSSamplingParams sampling_params)
    : m_envelope_scores(std::move(envelope_scores)),
      m_score_to_segmentation_strategy(std::move(score_to_segmentation_strategy)) {
    initialize(sampling_params);
}

void ScoreBasedChSegmentationStrategy::initialize_segmentation_strategies() {
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
                if (!m_envelope_scores->update(envelope)) early_stop = true;
            }
        }
    }

    m_segmentation_strategies =
        m_score_to_segmentation_strategy->get_segmentation_strategies(m_envelope_scores->get_scores());
}
