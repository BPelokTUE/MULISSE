#include "Index/Estimator/Sampling/EnvelopeQueryTimeParamEstimator.hpp"

#include "Enums/SearchMethodType.hpp"
#include "Index/Entry/SaxEnvelope.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Index/Index.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Modules/Indexing/ConstructIndex.hpp"
#include "Modules/Indexing/IndexFactory/GetFlatEnvelopeIndex.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Search/IndexSearch/FlatEnvelopeIndexSearch.hpp"
#include "Search/IndexSearch/LengthGroupingIndexSearch.hpp"
#include "Search/Results/ResultSet.hpp"
#include "Search/SearchMethod.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/RunSettings/RunSettings.hpp"

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

EnvelopeParams EnvelopeQueryTimeParamEstimator::get_estimated_params(
    const IndexOptions &index_opts, const IEnvelopeConfigGenerator *env_config_generator) {
    m_queries.resize(index_opts.m_estimator_params->m_sampling_params->m_num_queries,
                     vec<vec<Real>>(index_opts.m_num_channels));
    return EnvelopeSamplingParamEstimator::get_estimated_params(index_opts, env_config_generator);
}

void EnvelopeQueryTimeParamEstimator::update_queries(std::stringstream &query_stream, uint num_queries) {
    query_stream.seekg(0);
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(m_queries[0].size());

    for (uint q = 0; q < num_queries; ++q) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            Real query_sum = R(0.0), query_sq_sum = R(0.0);

            str line;
            std::getline(query_stream, line);
            std::stringstream channel_stream(line);
            {
                Real value;
                while (channel_stream >> value) {
                    m_queries[q][c].push_back(value);
                    query_sum += value;
                    query_sq_sum += value * value;
                }
            }
            if (m_queries[q][c].empty()) continue;

            auto [mu, sigma] = calculate_mu_and_sigma(query_sum, query_sq_sum, U(m_queries[q][c].size()));
            for (uint i = 1; i < m_queries[q][c].size(); ++i) {
                m_queries[q][c][i] = (m_queries[q][c][i] - mu) / sigma;
            }
        }
    }
}

Real EnvelopeQueryTimeParamEstimator::get_config_score(
    vec<vec<IndexEntry<Envelope>>> &&entries, const IndexOptions &config_opts, const LengthProperties &length_props,
    const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) {
    // 1. Create index factory using the type of index method in the the index options
    std::function<sptr<IIndex<Envelope>>(IndexFactoryParams &)> index_factory;
    std::function<uptr<ISearchMethod<KNN, ED>>(uptr<IFinalizedIndex<EnvelopeTag>>)> search_method_factory;

    switch (config_opts.m_index_method) {
        case ENVELOPE:
            index_factory = get_flat_envelope_index<Envelope>;
            search_method_factory = [](uptr<IFinalizedIndex<EnvelopeTag>> index) {
                return std::make_unique<FlatEnvelopeIndexSearch<Envelope, KNN, ED, false, false>>(
                    uptr<FinalizedFlatEnvelopeIndex<Envelope>>(
                        static_cast<FinalizedFlatEnvelopeIndex<Envelope> *>(index.release())));
            };
            break;
        case SAX_ENVELOPE:
            index_factory = get_flat_envelope_index<SaxEnvelope>;
            search_method_factory = [](uptr<IFinalizedIndex<EnvelopeTag>> index) {
                return std::make_unique<FlatEnvelopeIndexSearch<SaxEnvelope, KNN, ED, false, false>>(
                    uptr<FinalizedFlatEnvelopeIndex<SaxEnvelope>>(
                        static_cast<FinalizedFlatEnvelopeIndex<SaxEnvelope> *>(index.release())));
            };
            break;
        default:
            throw std::runtime_error("EnvelopeQueryTimeParamEstimator cannot be used with index method " +
                                     SEARCH_METHOD_TYPE_TO_STR.at(config_opts.m_index_method));
    }

    // 2. Create index and fill with data
    auto index = get_index_without_data<Envelope>(index_factory, config_opts, lg_segmentation_strategy);
    index->insert_entry_groups(entries, config_opts.m_inserter_type);
    auto finalized_index = index->finalize();

    // 3. Create search method
    uptr<ISearchMethod<KNN, ED>> search_method;
    if (config_opts.m_use_length_groups) {
        vec<uptr<ISearchMethod<KNN, ED>>> search_methods(length_props.m_num_l_groups);
        auto grouping_index = uptr<FinalizedLengthGroupingIndex<EnvelopeTag>>(
            static_cast<FinalizedLengthGroupingIndex<EnvelopeTag> *>(finalized_index.release()));
        for (uint l_ind = 0; l_ind < length_props.m_num_l_groups; l_ind++) {
            search_methods[l_ind] =
                search_method_factory(uptr<IFinalizedIndex<EnvelopeTag>>(grouping_index->release_index(l_ind)));
        }
        search_method = std::make_unique<LengthGroupingIndexSearch<KNN, ED>>(std::move(search_methods), length_props);
    } else {
        search_method = search_method_factory(std::move(finalized_index));
    }

    // 4. Set up arguments for search
    uint knn_k = 1;
    bool normalized = config_opts.m_normalized, use_early_abandoning = true;
    ResultSet<KNN> result_set(knn_k);
    DistanceMeasure<KNN, ED> distance_measure(normalized, use_early_abandoning);
    SearchOptions search_opts{
        .m_normalized = normalized,
        .m_use_early_abandoning = use_early_abandoning,
        .m_search_method_type = config_opts.m_index_method,
        .m_search_type = KNN,
        .m_distance_type = ED,
        .m_knn_k = knn_k,
    };
    std::ifstream data_stream(RunSettings::get_instance().get_dataset_path(), std::ios::binary);

    // 5. Run queries and calculate average query time
    double total_time = 0.0;
    for (auto &query : m_queries) {
        TimePoint start_time = std::chrono::high_resolution_clock::now();
        search_method->search(query, search_opts, result_set, distance_measure, data_stream);
        TimePoint end_time = std::chrono::high_resolution_clock::now();
        total_time += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    }
    return -R(total_time / static_cast<double>(m_queries.size()));
}
