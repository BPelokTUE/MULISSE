#ifndef SEARCHING_HPP
#define SEARCHING_HPP

#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"
#include "Search/LengthGroupingIndex.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/ChainIndex.hpp"
#include "Search/SequentialScan.hpp"

/**
 * @brief Helper function for loading index-based search methods
 * @tparam FTag The traits of the entries in the index
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the method takes sorted queries
 * @param finalized_index_factory Function to create the finalized index
 * @param search_method_factory Function to create the search method
 * @param opts Options for searching
 * @return Pointer to the search method
 */
template <typename FTag, SearchType S, DistanceType D, bool QS = false>
    requires ValidEntryTraitsTag<FTag>
uptr<ISearchMethod<S, D, QS>> load_index_based_method(
    std::function<uptr<IFinalizedIndex<FTag>>()> finalized_index_factory,
    std::function<uptr<ISearchMethod<S, D, QS>>(uptr<IFinalizedIndex<FTag>>)> search_method_factory,
    const SearchOptions &opts) {
    auto &RS = RunSettings::get_instance();

    uptr<IFinalizedIndex<FTag>> index;
    uint series_len = RS.get_dataset_props().series_len;
    uint num_len_groups = opts.lens_per_group > 0 ? (series_len + opts.lens_per_group - 1) / opts.lens_per_group : 0;

    vec<uptr<IFinalizedIndex<FTag>>> group_indexes(num_len_groups);
    vec<uptr<ISearchMethod<S, D, QS>>> search_methods(num_len_groups);

    if (num_len_groups > 0) {
        for (uint l_ind = 0; l_ind < num_len_groups; l_ind++) {
            group_indexes[l_ind] = finalized_index_factory();
        }
        index = std::make_unique<LengthGroupingFinalizedIndex<FTag>>(std::move(group_indexes), series_len);
    } else {
        index = finalized_index_factory();
    }

    index->load(RS.get_index_path(), opts.index_format);

    if (num_len_groups > 0) {
        for (uint l_ind = 0; l_ind < num_len_groups; l_ind++) {
            search_methods[l_ind] = search_method_factory(std::move(group_indexes[l_ind]));
        }
        return std::make_unique<LengthGroupingIndexSearch<S, D, QS>>(std::move(search_methods), series_len);
    } else {
        return search_method_factory(std::move(index));
    }
}

/**
 * @brief Load the search method based on the options
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the method takes sorted queries
 * @param opts Options for searching
 * @return Pointer to the search method
 */
template <SearchType S, DistanceType D, bool QS>
uptr<ISearchMethod<S, D, QS>> load_method(const SearchOptions &opts) {
    auto &RS = RunSettings::get_instance();

    switch (opts.search_method_type) {
        case ISAX_ENVELOPE:
            return load_index_based_method<EnvelopeTag, S, D, QS>(
                []() { return std::make_unique<iSaxFinalizedIndex<EnvelopeTag>>(); },
                [](uptr<IFinalizedIndex<EnvelopeTag>> index) {
                    return std::make_unique<iSaxIndexSearch<EnvelopeTag, S, D, QS>>(
                        uptr<iSaxFinalizedIndex<EnvelopeTag>>(
                            static_cast<iSaxFinalizedIndex<EnvelopeTag> *>(index.release())));
                },
                opts);
        case ISAX_ENV_W_ENV:
        case ISAX_ENV_W_SAX_ENV:
            return load_index_based_method<EnvelopeTag, S, D, QS>(
                []() {
                    vec<uptr<IFinalizedIndex<EnvelopeTag>>> approx_indexes(1);
                    approx_indexes[0] = std::make_unique<iSaxFinalizedIndex<EnvelopeTag>>();
                    auto exact_index = uptr<IFinalizedIndex<EnvelopeTag>>(new iSaxFinalizedIndex<EnvelopeTag>());
                    return std::make_unique<ChainFinalizedIndex<EnvelopeTag>>(std::move(approx_indexes),
                                                                              std::move(exact_index));
                },
                [](uptr<IFinalizedIndex<EnvelopeTag>> index) {
                    auto chain_index = uptr<ChainFinalizedIndex<EnvelopeTag>>(
                        static_cast<ChainFinalizedIndex<EnvelopeTag> *>(index.release()));
                    vec<uptr<ISearchMethod<S, D, QS>>> approx_methods(1);
                    approx_methods[0] =
                        std::make_unique<iSaxIndexSearch<EnvelopeTag, S, D, QS>>(uptr<iSaxFinalizedIndex<EnvelopeTag>>(
                            static_cast<iSaxFinalizedIndex<EnvelopeTag> *>(chain_index->release_approx_index(0))));
                    auto exact_method = std::make_unique<FlatEnvelopeIndexSearch<S, D, QS>>(
                        uptr<FlatEnvelopeIndex>(static_cast<FlatEnvelopeIndex *>(chain_index->release_exact_index())));
                    return std::make_unique<ChainSearch<S, D, QS>>(std::move(approx_methods), std::move(exact_method));
                },
                opts);
        case ISAX:
            load_index_based_method<PaaTag, S, D, QS>(
                []() { return std::make_unique<iSaxFinalizedIndex<PaaTag>>(); },
                [](uptr<IFinalizedIndex<PaaTag>> index) {
                    return std::make_unique<iSaxIndexSearch<PaaTag, S, D, QS>>(
                        uptr<iSaxFinalizedIndex<PaaTag>>(static_cast<iSaxFinalizedIndex<PaaTag> *>(index.release())));
                },
                opts);
        case ENVELOPE:
        case SAX_ENVELOPE: {
            load_index_based_method<EnvelopeTag, S, D, QS>(
                []() { return std::make_unique<FlatEnvelopeIndex>(); },
                [opts](uptr<IFinalizedIndex<EnvelopeTag>> index) {
                    return std::make_unique<FlatEnvelopeIndexSearch<S, D, QS>>(
                        uptr<FlatEnvelopeIndex>(static_cast<FlatEnvelopeIndex *>(index.release())),
                        opts.use_priority_queue);
                },
                opts);
        }
        case SEQUENTIAL_SCAN:
            return std::make_unique<SequentialScan<S, D, QS>>();
    }
    return nullptr;
}

/**
 * @brief Execute similarity search
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether to sort the query or not
 * @param opts Options for searching
 */
template <SearchType S, DistanceType D, bool QS = false>
int search(const SearchOptions &opts, ResultSet<S> &result_set, DistanceMeasure<S, D, QS> &distance_measure) {
    uptr<ISearchMethod<S, D, QS>> method = load_method<S, D, QS>(opts);

    if (!method) return 1;

    auto &RS = RunSettings::get_instance();
    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    std::ifstream query_ifs(RS.get_query_path());

    QueryLogger::initialize(opts);
    auto &logger = QueryLogger::get_instance();

    MtsNumChannelsT num_channels = RunSettings::get_instance().get_dataset_props().num_channels;
    vec<vec<Real>> query(num_channels);

    size_t query_count = 0, query_len = 0;
    for (MtsNumChannelsT c = 0; !query_ifs.eof(); c = (c + 1) % num_channels) {
        str line;
        std::getline(query_ifs, line);
        std::istringstream iss(line);
        Real value, sum = 0, sq_sum = 0;

        query[c].clear();
        while (iss >> value) {
            query[c].push_back(value);
            sum += value;
            sq_sum += value * value;
        }
        if (!query[c].empty()) {
            query_len = query[c].size();
            if (opts.normalized) {
                auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, query[c].size());
                for (size_t i = 0; i < query[c].size(); ++i) query[c][i] = (query[c][i] - mu) / sigma;
            }
        }

        if (c == num_channels - 1) {
            logger.reset_entry();
            logger.set_number_col(QC::QUERY_ID, query_count++);
            logger.log_query(query);

            dataset_ifs.seekg(0);
            result_set.clear();
            if (D == MASS) {
                fftwr_forget_wisdom();
                fftwr_cleanup();
                if (RS.ffts_supported()) RS.reset_query_ffts();
            }

            logger.start_timer(QC::TOTAL_TIME_S);
            SearchResults results;
            if constexpr (QS && D == ED) {
                vec<std::pair<Real, uint>> query_magnitudes(query_len);
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    if (query[c].empty()) continue;
                    for (uint i = 0; i < query_len; ++i) {
                        query_magnitudes[i].first += std::abs(query[c][i]);
                        query_magnitudes[i].second = i;
                    }
                }
                std::sort(query_magnitudes.begin(), query_magnitudes.end(), std::greater<std::pair<Real, uint>>());

                vec<uint> real_query_inds(query_len);
                vec<vec<Real>> sorted_query(num_channels, vec<Real>(query_len));

                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    if (query[c].empty()) continue;
                    for (uint i = 0; i < query_len; ++i) {
                        real_query_inds[i] = query_magnitudes[i].second;
                        sorted_query[c][i] = query[c][real_query_inds[i]];
                    }
                }

                results =
                    method->search(sorted_query, opts, result_set, distance_measure, dataset_ifs, &real_query_inds);
            } else {
                results = method->search(query, opts, result_set, distance_measure, dataset_ifs);
            }
            logger.stop_timer(QC::TOTAL_TIME_S);
            logger.log_results(results);
            logger.write_entry();
        }
    }
    return 0;
}

#endif  // SEARCHING_HPP
