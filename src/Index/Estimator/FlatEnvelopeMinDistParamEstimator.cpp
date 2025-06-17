#include "Index/Estimator/FlatEnvelopeMinDistParamEstimator.hpp"

#include <cassert>
#include <random>

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/EntryMerger/DummyEntryMerger.hpp"
#include "Index/Estimator/ConfigGenerator/DummyConfigGenerator.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Index/Summarization.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"
#include "Modules/QueryGen.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Search/QuerySetOptions.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"
#include "Util/RunSettings/RunSettings.hpp"

FlatEnvelopeMinDistParamEstimator::FlatEnvelopeMinDistParamEstimator(const IndexOptions &opts) {
    // 1. Generate configurations
    // 2. Sample data
    // 3. Create queries from data
    // 4. For each configuration:
    //     4.1. Create a FlatEnvelopeIndex, skipping positions and lengths in the envelopes
    //     4.2. Measure the average lower-bound distance for the queries
    // 5. Select the configuration with the lowest average lower-bound distance

    uint seed = 211;             // TMP
    uint last_ind_step = 10;     // TMP
    uint first_ind_step = 10;    // TMP
    Real sample_frac = R(0.05);  // TMP
    uint num_queries = 10;       // TMP

    auto &RS = RunSettings::get_instance();

    // 1. Generate configurations
    vec<FlatEnvelopeParams> configurations =
        DummyConfigGenerator().generate_configurations(opts.m_estimator_params->m_index_size_limit);

    // 2. Sample data
    auto [num_channels, series_len, num_series, dataset_file] = RS.get_dataset_props();
    vec<uint> mts_inds(num_series);
    std::iota(mts_inds.begin(), mts_inds.end(), 0);

    assert(sample_frac >= 0.0 && sample_frac <= 1.0);
    if (sample_frac < 1.0) {
        num_series = U(R(num_series) * sample_frac);
        std::shuffle(mts_inds.begin(), mts_inds.end(), std::mt19937{std::random_device{}()});
    }

    // 3. Create queries from data
    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;
    str dataset_path = RS.get_dataset_path();

    vec<vec<vec<Real>>> query_accs(num_queries, vec<vec<Real>>(num_channels));
    {
        std::ifstream data_stream(dataset_path, std::ios::binary);
        std::stringstream query_stream;
        QuerySetOptions query_opts{
            .m_noise = R(0.1),
            .m_num_queries = num_queries,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_used_channels = opts.m_num_channels,
            .m_seed = seed,
        };
        generate_queries(data_stream, query_stream, query_opts, mts_inds);

        query_stream.seekg(0);
        for (uint q = 0; q < num_queries; ++q) {
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                Real query_sum = R(0.0), query_sq_sum = R(0.0);

                str line;
                std::getline(query_stream, line);
                std::stringstream channel_stream(line);
                {
                    Real value;
                    while (channel_stream >> value) {
                        query_accs[q][c].push_back(value);
                        query_sum += value;
                        query_sq_sum += value * value;
                    }
                }
                if (query_accs[q][c].empty()) continue;

                auto [mu, sigma] = calculate_mu_and_sigma(query_sum, query_sq_sum, U(query_accs[q][c].size()));
                query_accs[q][c][0] = (query_accs[q][c][0] - mu) / sigma;
                for (uint i = 1; i < query_accs[q][c].size(); ++i) {
                    query_accs[q][c][i] = query_accs[q][c][i - 1] + (query_accs[q][c][i] - mu) / sigma;
                }
            }
        }
    }

    // Get envelope params
    const EnvelopeIndexParams *params_ptr = dynamic_cast<const EnvelopeIndexParams *>(opts.m_index_params.get());
    if (!params_ptr) throw std::runtime_error("FlatEnvelopeMinDistParamEstimator requires EnvelopeIndexParams.");

    // Distance measure for min distance calculation
    DistanceMeasure<KNN, ED> distance_measure(opts.m_normalized);

    // Configuration minimum distance totals
    vec<Real> min_dist_totals(configurations.size(), R(0.0));
    Real max_min_dist_total = R(0.0);
    uint selected_config_ind = 0;

    // 4. For each configuration
    for (uint config_ind = 0; config_ind < configurations.size(); ++config_ind) {
        auto &config = configurations[config_ind];

        auto envelope_index_params = std::make_unique<EnvelopeIndexParams>(*params_ptr);
        envelope_index_params->m_pos_per_env = config.m_pos_per_env;
        envelope_index_params->m_segmentation_params.m_num_segments = config.m_num_segments;
        IndexOptions config_opts{
            .m_normalized = opts.m_normalized,
            .m_use_length_groups = true,
            .m_num_channels = opts.m_num_channels,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_series_len = series_len,
            .m_l_per_group = config.m_l_per_group,
            .m_index_params = std::move(envelope_index_params),
        };
        auto lg_segmentation_strategy = get_lg_segmentation_strategy(opts);
        EnvelopeParams env_params{
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_pos_per_env = config.m_pos_per_env,
            .m_lg_segmentation_strategy = lg_segmentation_strategy.get(),
        };
        uint num_l_groups = (l_max - l_min + config.m_l_per_group) / config.m_l_per_group;
        LengthProperties length_props{
            .m_use_length_groups = true,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_l_per_group = config.m_l_per_group,
            .m_num_l_groups = num_l_groups,
        };
        auto generator = std::make_unique<EnvelopeEntryGenerator>(opts.m_normalized, env_params, num_l_groups,
                                                                  last_ind_step, first_ind_step);
        auto merger = std::make_unique<DummyEntryMerger<Envelope>>();

        // 4.1. Create a FlatEnvelopeIndex, skipping positions and lengths in the envelopes
        auto entries =
            summarize_dataset<Envelope>(num_l_groups, num_series, mts_inds, std::move(generator), std::move(merger));

        // 4.2. Measure the average lower-bound distance for the queries
        for (auto &query_acc : query_accs) {
            uint query_len = U(query_acc[0].size());  // TODO: fix this in case not all channels are used

            uint lg_ind = length_props.get_length_group(query_len);
            auto ch_segmentation_strategy = lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind);

            Real min_dist_sum_query = R(0.0);
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                auto segmentation_strategy = ch_segmentation_strategy->get_const_segmentation_strategy(c);
                vec<Real> query_ch_paa(segmentation_strategy->get_num_segments(query_len));

                uint seg_start = 0;
                for (SaxSegIndT seg_ind = 0; seg_ind < query_ch_paa.size(); ++seg_ind) {
                    uint seg_len = segmentation_strategy->get_segment_len(seg_ind), seg_end = seg_start + seg_len;
                    query_ch_paa[seg_ind] = (query_acc[c][seg_end] - query_acc[c][seg_start]) / R(seg_len);
                    seg_start = seg_end;
                }

                for (auto &entry : entries[lg_ind]) {
                    for (SaxSegIndT seg_ind = 0; seg_ind < entry.m_mts_summary.size(); ++seg_ind) {
                        min_dist_sum_query += distance_measure.min_dist_squared(
                            query_ch_paa[seg_ind], entry.m_mts_summary[c].m_lower[seg_ind],
                            entry.m_mts_summary[c].m_upper[seg_ind]);
                    }
                }
                min_dist_totals[config_ind] += min_dist_sum_query / R(entries[lg_ind].size());
            }
        }
        if (min_dist_totals[config_ind] > max_min_dist_total) {
            max_min_dist_total = min_dist_totals[config_ind];
            selected_config_ind = config_ind;
        }
    }
    m_estimated_params = configurations[selected_config_ind];
}

FlatEnvelopeParams FlatEnvelopeMinDistParamEstimator::get_estimated_params() { return m_estimated_params; }
