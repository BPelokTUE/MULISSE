#ifndef SEARCH_SEARCHMETHOD_HPP
#define SEARCH_SEARCHMETHOD_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <SearchType S, DistanceType D, bool SQ>
class DistanceMeasure;

template <SearchType S>
class ResultSet;

class SearchOptions;
class SearchResults;

/**
 * @brief Interface for search methods
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam SQ Whether the query is sorted or not
 */
template <SearchType S, DistanceType D, bool SQ = false>
class ISearchMethod {
   public:
    virtual ~ISearchMethod() = default;

    /**
     * @brief Search for multivariate subsequence
     * @param query Multivariate subsequence to search for
     * @param search_options Search options
     * @param result_set Set for managing the results
     * @param distance_measure Distance measure to use
     * @param dataset_ifs Input file stream for the dataset
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @return The start positions of the subsequences in the result set
     */
    virtual SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                                 const DistanceMeasure<S, D, SQ> &distance_measure, std::ifstream &dataset_ifs,
                                 const vec<uint> *real_query_inds = nullptr) = 0;

    /** @brief Reset the search method after a query. The default implementation does nothing. */
    virtual inline void reset() {}

   protected:
    /**
     * @brief Get the Paa values and the length of the query
     * @param query The multivariate query
     * @param ch_segmentation_strategy The segmentation strategy to use
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @param normalized Whether the subsequences are Z-normalized
     * @return A pair containing the Paa values and the length of the query
     */
    inline std::pair<vec<vec<Real>>, uint> get_query_paa_and_len(
        const vec<vec<Real>> &query, const IChannelSegmentationStrategy *ch_segmentation_strategy,
        const vec<uint> *real_query_inds = nullptr, bool normalized = true) const {
        auto &RS = RunSettings::get_instance();

        vec<vec<Real>> query_paa(query.size());
        size_t query_len = 0;

        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(query.size());
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (query[c].empty()) continue;
            query_len = std::max(query_len, query[c].size());

            if constexpr (SQ) {
                vec<Real> unsorted_query_channel(query_len);
                for (uint i = 0; i < query_len; ++i) unsorted_query_channel[real_query_inds->at(i)] = query[c][i];
                query_paa[c] =
                    paa(unsorted_query_channel, ch_segmentation_strategy->get_const_segmentation_strategy(c));
            } else {
                query_paa[c] = paa(query[c], ch_segmentation_strategy->get_const_segmentation_strategy(c));
            }
            if (!normalized) {
                auto [mu, sigma] = RS.get_channel_mean_and_std(c);
                for (Real &value : query_paa[c]) value = (value - mu) / sigma;
            }
        }
        return {query_paa, query_len};
    }
};

#endif  // SEARCH_SEARCHMETHOD.HPP
