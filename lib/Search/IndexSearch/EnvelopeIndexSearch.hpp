#ifndef SEARCH_INDEXSEARCH_ENVELOPEINDEXSEARCH_HPP
#define SEARCH_INDEXSEARCH_ENVELOPEINDEXSEARCH_HPP

#include "Search/IndexSearch/IndexSearchMethod.hpp"
#include "Util/Logging/QueryLogger.hpp"

/**
 * @brief Abstract base class for envelope-index-based search methods
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the method takes sorted queries
 * */
template <SearchType S, DistanceType D, bool QS = false>
class EnvelopeIndexSearch : public IndexSearchMethod<EnvelopeTag, S, D, QS> {
   protected:
    /**
     * @brief Get the minimum bounding distance squared between the given query and envelope
     * @param envelope The multivariate envelope
     * @param query_paa The query paa
     * @param distance_measure The distance measure to use
     * @param ch_segmentation_strategy The segmentation strategy to use
     * @return The minimum bounding distance squared
     */
    inline Real get_min_dist_squared(const vec<Envelope> &envelope, const vec<vec<Real>> &query_paa,
                                     const DistanceMeasure<S, D, QS> &distance_measure,
                                     const IChannelSegmentationStrategy *ch_segmentation_strategy) const {
        Real min_dist_squared = 0;
        for (MtsNumChannelsT c = 0; c < query_paa.size(); ++c) {
            auto segmentation_strategy = ch_segmentation_strategy->get_segmentation_strategy(c);
            for (SaxSegIndT s = 0; s < query_paa[c].size(); ++s) {
                Real segment_len_r = R(segmentation_strategy->get_segment_len(s));
                min_dist_squared +=
                    distance_measure.min_dist_squared(query_paa[c][s], envelope[c].m_lower[s], envelope[c].m_upper[s]) *
                    segment_len_r;
            }
        }
        return min_dist_squared;
    }

    /**
     * @brief Update the result set with the exact distances to the entries in the specified subsequence
     * @param subs_info The subsequence information
     * @param pos_per_env The number of positions per envelope
     * @param query The multivariate query
     * @param query_len The length of the query
     * @param result_set The result set to update
     * @param distance_measure The distance measure to use
     * @param dataset_ifs The input file stream for the dataset
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     */
    inline void update_result_set(const SubsequenceInfo &subs_info, const uint pos_per_env, const vec<vec<Real>> &query,
                                  const uint query_len, ResultSet<S> &result_set,
                                  const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                                  const vec<uint> *real_query_inds) const {
        auto &logger = QueryLogger::get_instance();

        auto &RS = RunSettings::get_instance();
        uint series_len = RS.get_dataset_props().m_series_len;
        MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;

        vec<vec<Real>> subsequence(num_channels);
        uint num_start_pos = subs_info.m_length;
        size_t data_to_read = std::min(query_len + num_start_pos - 1, series_len - subs_info.m_start_pos);

        logger.start_timer(QC::IO_TIME_S);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (query[c].empty()) continue;

            subsequence[c].resize(data_to_read);
            dataset_ifs.seekg(subs_info.get_file_pos(series_len, num_channels, c));
            dataset_ifs.read(reinterpret_cast<char *>(subsequence[c].data()),
                             static_cast<std::streamsize>(data_to_read * sizeof(Real)));
        }
        logger.stop_timer(QC::IO_TIME_S);

        logger.start_timer(QC::TS_EXAMINATION_TIME_S);
        distance_measure.update_result_set(result_set, subs_info, query, subsequence, real_query_inds);
        logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

        logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED);
    }
};

#endif  // SEARCH_INDEXSEARCH_ENVELOPEINDEXSEARCH_HPP
