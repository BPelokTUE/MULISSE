#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScoreEstimator.hpp"

#include <algorithm>
#include <fstream>
#include <random>

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeScoreFunc.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSSParams.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreToSegmentationStrategy.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Pointers.hpp"

EnvelopeScoreEstimator::EnvelopeScoreEstimator(uptr<IEnvelopeScoreFunc> envelope_scores)
    : m_envelope_scores(std::move(envelope_scores)) {}

vec<Real> EnvelopeScoreEstimator::estimate_scores(const ScoreBasedChSSParams &score_based_chss_params) {
    // 1. Get dataset path
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();

    // 2. Get subset of dataset
    auto dataset_props = RS.get_dataset_props();
    MtsNumChannelsT num_channels = dataset_props.m_num_channels;
    uint series_len = dataset_props.m_series_len, num_series = dataset_props.m_num_series;

    vec<vec<uint>> mts_inds(num_channels, vec<uint>(score_based_chss_params.m_sample_size));
    auto mt = std::mt19937{std::random_device{}()};
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        vec<uint> channel_inds(num_series);
        std::iota(channel_inds.begin(), channel_inds.end(), 0);
        std::shuffle(channel_inds.begin(), channel_inds.end(), mt);
        mts_inds[c].assign(channel_inds.begin(), channel_inds.begin() + score_based_chss_params.m_sample_size);
    }

    // 3. Get generator

    auto &length_props = RS.get_length_props();
    uint l_min = length_props.m_l_min, l_max = length_props.m_l_max;
    auto segmentation_strategy =
        std::make_shared<UniformSegmentationStrategy>(l_max, l_max / score_based_chss_params.m_segment_len);
    auto ch_segmentation_strategy = std::make_shared<SingleChSegmentationStrategy>(std::move(segmentation_strategy));

    uint pos_per_env = series_len - l_min + 1;
    auto lg_segmentation_strategy = std::make_unique<SingleLGSegmentationStrategy>(std::move(ch_segmentation_strategy));
    EnvelopeParams envelope_params{.m_l_min = l_min,
                                   .m_l_max = l_max,
                                   .m_pos_per_env = pos_per_env,
                                   .m_lg_segmentation_strategy = lg_segmentation_strategy.get()};

    auto generator = std::make_unique<EnvelopeEntryGenerator>(score_based_chss_params.m_normalized, envelope_params);

    // 4. Initialize segmentation strategies according to the implementation
    uint original_l_per_group = length_props.m_l_per_group;
    RS.set_lengths_per_group(l_max - l_min + 1);  // Ugly hack

    volatile bool early_stop = false;

    OMP_PRAGMA(omp parallel) {
        std::ifstream dataset_ifs(dataset_path, std::ios::binary);
        OMP_PRAGMA(omp for)
        for (uint i = 0; i < score_based_chss_params.m_sample_size; ++i) {
            if (early_stop) continue;

            vec<vec<Real>> mts(num_channels, vec<Real>(series_len));
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                uint mts_ind = mts_inds[c][i];
                dataset_ifs.seekg((mts_ind * num_channels + c) * series_len * sizeof(Real));
                dataset_ifs.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(Real));
            }
            auto envelope = generator->get_entries(mts, 0)[0][0].m_mts_summary;
            OMP_PRAGMA(omp critical) {
                if (!m_envelope_scores->update(envelope)) early_stop = true;
            }
        }
    }

    RS.set_lengths_per_group(original_l_per_group);  // Ugly hack

    return m_envelope_scores->get_scores();
}

vec<Real> EnvelopeScoreEstimator::get_scores() const { return m_envelope_scores->get_scores(); }
