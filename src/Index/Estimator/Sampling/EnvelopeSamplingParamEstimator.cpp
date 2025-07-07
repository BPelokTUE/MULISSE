#include "Index/Estimator/Sampling/EnvelopeSamplingParamEstimator.hpp"

#include <cassert>
#include <random>

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryGenerator/EnvelopeEntryGenerator.hpp"
#include "Index/EntryMerger/DummyEntryMerger.hpp"
#include "Index/Estimator/EnvelopeConfigGenerator/GridEnvConfigGenerator.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Index/Summarization.hpp"
#include "Modules/Indexing/StrategyFactory/GetLGSegmentationStrategy.hpp"
#include "Modules/QueryGen.hpp"
#include "Search/QuerySetOptions.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"
#include "Util/Logging/ParamEstimatesLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

EnvelopeSamplingParamEstimator::EnvelopeSamplingParamEstimator(const IndexOptions &index_opts) {
    if (!index_opts.m_estimator_params || !index_opts.m_estimator_params->m_sampling_params) {
        throw std::runtime_error("EnvelopeSamplingParamEstimator requires sampling parameters.");
    }
}

EnvelopeParams EnvelopeSamplingParamEstimator::get_estimated_params() { return m_estimated_params; }

void EnvelopeSamplingParamEstimator::estimate_params(const IndexOptions &index_opts) {
    // 1. Generate configurations
    // 2. Sample data
    // 3. Create queries from data
    // 4. For each configuration:
    //     4.1. Create a FlatEnvelopeIndex, skipping positions and lengths in the envelopes
    //     4.2. Measure the average lower-bound distance for the queries
    // 5. Select the configuration with the lowest average lower-bound distance

    auto &RS = RunSettings::get_instance();
    auto &logger = ParamEstimatesLogger::get_instance();
    auto &sampling_params = *(index_opts.m_estimator_params->m_sampling_params);

    // 1. Generate configurations
    vec<EnvelopeParams> configurations = GridEnvConfigGenerator().generate_configurations(
        index_opts.m_index_method, index_opts.m_estimator_params->m_index_size_limit);

    // 2. Sample data
    auto [num_channels, series_len, num_series, dataset_file] = RS.get_dataset_props();
    m_mts_inds.resize(num_series);
    std::iota(m_mts_inds.begin(), m_mts_inds.end(), 0);

    assert(sampling_params.m_sample_frac >= 0.0 && sampling_params.m_sample_frac <= 1.0);
    if (sampling_params.m_sample_frac < 1.0) {
        num_series = U(R(num_series) * sampling_params.m_sample_frac);
        std::shuffle(m_mts_inds.begin(), m_mts_inds.end(), std::mt19937{std::random_device{}()});
    }

    // 3. Create queries from data
    uint l_min = RS.get_length_props().m_l_min, l_max = RS.get_length_props().m_l_max;
    str dataset_path = RS.get_dataset_path();
    {
        std::ifstream data_stream(dataset_path, std::ios::binary);
        std::stringstream query_stream;
        QuerySetOptions query_opts{
            .m_noise = R(0.1),
            .m_num_queries = sampling_params.m_num_queries,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_used_channels = index_opts.m_num_channels,
            .m_seed = sampling_params.m_seed,
        };
        generate_queries(data_stream, query_stream, query_opts, m_mts_inds);
        update_queries(query_stream, sampling_params.m_num_queries);
    }

    // Get envelope params
    const EnvelopeIndexParams *params_ptr = dynamic_cast<const EnvelopeIndexParams *>(index_opts.m_index_params.get());
    if (!params_ptr) throw std::runtime_error("EnvelopeMinDistParamEstimator requires EnvelopeIndexParams.");

    // 4. For each configuration
    Real max_config_score = -INF;
    for (uint config_ind = 0; config_ind < configurations.size(); ++config_ind) {
        auto &config = configurations[config_ind];

        auto envelope_index_params = std::make_unique<EnvelopeIndexParams>(*params_ptr);
        envelope_index_params->m_pos_per_env = config.m_pos_per_env;
        envelope_index_params->m_segmentation_params.m_num_segments = config.m_num_segments;
        IndexOptions config_opts{
            .m_normalized = index_opts.m_normalized,
            .m_use_length_groups = true,
            .m_num_channels = index_opts.m_num_channels,
            .m_index_method = index_opts.m_index_method,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_series_len = series_len,
            .m_l_per_group = config.m_l_per_group,
            .m_index_params = std::move(envelope_index_params),
        };
        auto lg_segmentation_strategy = get_lg_segmentation_strategy(config_opts);
        uint num_l_groups = (l_max - l_min + config.m_l_per_group) / config.m_l_per_group;
        LengthProperties length_props{
            .m_use_length_groups = true,
            .m_l_min = l_min,
            .m_l_max = l_max,
            .m_l_per_group = config.m_l_per_group,
            .m_num_l_groups = num_l_groups,
        };
        auto generator = std::make_unique<EnvelopeEntryGenerator>(
            config_opts.m_normalized, config.m_pos_per_env, length_props, lg_segmentation_strategy.get(),
            sampling_params.m_ind_step, sampling_params.m_ind_step);
        auto merger = std::make_unique<DummyEntryMerger<Envelope>>();

        // 4.1. Create a FlatEnvelopeIndex, skipping positions and lengths in the envelopes
        auto entries =
            summarize_dataset<Envelope>(num_l_groups, num_series, m_mts_inds, std::move(generator), std::move(merger));

        // 4.2. Update selected configuration if current is better
        Real config_score =
            get_config_score(std::move(entries), config_opts, length_props, lg_segmentation_strategy.get());
        if (config_score > max_config_score) {
            m_estimated_params = config;
            max_config_score = config_score;
        }

        logger.write_entry(config, config_score);
    }
}
