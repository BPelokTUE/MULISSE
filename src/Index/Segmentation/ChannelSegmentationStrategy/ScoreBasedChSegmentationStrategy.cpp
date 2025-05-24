#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBasedChSegmentationStrategy.hpp"

#include <algorithm>
#include <random>

#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/EnvelopeIndex/Flat/FlatEnvelopeIndex.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/SingleChSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/SingleLGSegmentationStrategy.hpp"
#include "Index/Segmentation/ScoreToSegmentationStrategy/ScoreToSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/UniformSegmentationStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Stats/IndexStats.hpp"
#include "Util/Stats/ScoreFunc/IndexStatsScoreFunc.hpp"
#include "Util/Types/Pointers.hpp"

ScoreBasedChSegmentationStrategy::ScoreBasedChSegmentationStrategy(
    const IndexStatsScoreFunc *index_stats_score_func,
    const IScoreToSegmentationStrategy *score_to_segmentation_strategy, Real subset_fraction, SaxSegIndT segment_len) {
    // Steps:
    // 1. Get dataset reader
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();

    // 2. Get subset of dataset
    auto dataset_props = RS.get_dataset_props();
    MtsNumChannelsT num_channels = dataset_props.m_num_channels;
    uint series_len = dataset_props.m_series_len, num_series = dataset_props.m_num_series;
    uint subset_size = U(R(num_series) * subset_fraction);

    std::vector<uint> mts_inds(num_series);
    std::iota(mts_inds.begin(), mts_inds.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(mts_inds.begin(), mts_inds.end(), g);

    // 3. Analyze envelopes
    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;
    auto segmentation_strategy = std::make_shared<UniformSegmentationStrategy>(l_max, l_max / segment_len);
    auto ch_segmentation_strategy = std::make_shared<SingleChSegmentationStrategy>(std::move(segmentation_strategy));

    uint pos_per_env = series_len - l_min + 1;
    auto lg_segmentation_strategy = std::make_unique<SingleLGSegmentationStrategy>(std::move(ch_segmentation_strategy));
    EnvelopeParams envelope_params{.m_l_min = l_min,
                                   .m_l_max = l_max,
                                   .m_pos_per_env = pos_per_env,
                                   .m_lg_segmentation_strategy = lg_segmentation_strategy.get()};

    uint original_l_per_group = RS.get_length_props().m_l_per_group;
    uint l_per_group = l_max - l_min + 1;
    RS.set_lengths_per_group(l_per_group);

    auto generator = std::make_unique<EnvelopeEntryGenerator>(true, envelope_params);

    vec<IndexStats> channel_stats(num_channels);
    OMP_PRAGMA(omp parallel) {
        std::ifstream dataset_ifs(dataset_path, std::ios::binary);
        OMP_PRAGMA(omp for)
        for (uint i = 0; i < subset_size; ++i) {
            uint mts_ind = mts_inds[i];
            dataset_ifs.seekg(mts_ind * series_len * num_channels * sizeof(Real));
            vec<vec<Real>> mts(num_channels, vec<Real>(series_len));
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                dataset_ifs.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(Real));
            }
            auto envelope = generator->get_entries(mts, U(mts_ind))[0][0].m_mts_summary;
            OMP_PRAGMA(omp critical) {
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    auto &summary = envelope[c];
                    for (SaxSegIndT s = 0; s < summary.size(); ++s) {
                        Real lower = summary.m_lower[s], upper = summary.m_upper[s];
                        channel_stats[c].update_seg_stats(lower, upper, c, s);
                    }
                }
            }
        }
    }
    RS.set_lengths_per_group(original_l_per_group);

    // 4. Get scores for each channel
    vec<Real> scores;
    scores.reserve(num_channels);
    for (auto &ch_stat : channel_stats) {
        ch_stat.calculate();
        scores.push_back(index_stats_score_func->calculate_score(ch_stat));
    }

    // 5. Get segmentation strategy for each channel
    m_segmentation_strategies = score_to_segmentation_strategy->get_segmentation_strategy(scores);
}

sptr<ISegmentationStrategy> ScoreBasedChSegmentationStrategy::get_segmentation_strategy(uint channel_ind) const {
    return m_segmentation_strategies[channel_ind];
}

const ISegmentationStrategy *ScoreBasedChSegmentationStrategy::get_const_segmentation_strategy(uint channel_ind) const {
    return m_segmentation_strategies[channel_ind].get();
}

ChannelSegmentationStrategyType ScoreBasedChSegmentationStrategy::get_type() const {
    return ChannelSegmentationStrategyType::SCORE_BASED;
}
