#ifndef SEARCH_DISTANCEMEASURE_EUCLIDEANDISTANCE_HPP
#define SEARCH_DISTANCEMEASURE_EUCLIDEANDISTANCE_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Search/DistanceMeasure/DistanceMeasure.hpp"

template <SearchType S, bool QS>
class DistanceMeasure<S, ED, QS> {
   public:
    /**
     * @brief Constructor
     * @param normalized Whether the time series are normalized
     * @param use_early_abandoning Whether to use early abandoning
     */
    DistanceMeasure(bool normalized, bool use_early_abandoning = true)
        : c_normalized(normalized), c_use_early_abandoning(use_early_abandoning) {}

    inline Real min_dist_squared(const Real paa, Real lower, Real upper) const {
        Real diff = upper < paa ? paa - upper : (lower > paa ? lower - paa : 0);
        return diff * diff;
    }

    inline bool update_result_set(ResultSet<S> &result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                                  const vec<vec<Real>> &mts, const vec<uint> *real_query_inds = nullptr) const {
        auto &logger = QueryLogger::get_instance();

        bool updated = false;
        uint num_start_pos, mts_len, query_len;
        vec<MtsNumChannelsT> present_channels;

        if constexpr (QS) {
            if (real_query_inds == nullptr) {
                throw std::runtime_error("No real query indices provided for sorted query");
            }
        }

        for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
            if (!(query[c].empty())) {
                query_len = U(query[c].size());
                mts_len = U(mts[c].size());
                assert(mts_len >= query_len);
                num_start_pos = U(mts_len - query_len + 1);
                present_channels.push_back(c);
            }
        }

        if (c_normalized) {
            vec<Real> sums(query.size()), sq_sums(query.size());
            for (MtsNumChannelsT c : present_channels) {
                for (size_t i = 0; i < query_len; ++i) {
                    sums[c] += mts[c][i];
                    sq_sums[c] += mts[c][i] * mts[c][i];
                }
            }

            for (uint start_pos = 0; start_pos < num_start_pos; ++start_pos) {
                Real dist_squared = 0;
                uint64_t points_examined = 0, point_in_entry = 0;

                for (MtsNumChannelsT c : present_channels) {
                    auto [mu, sigma] = calculate_mu_and_sigma(sums[c], sq_sums[c], query_len);

                    for (uint query_ind = 0; query_ind < query_len; ++query_ind) {
                        uint actual_ind = query_ind;
                        if constexpr (QS) actual_ind = real_query_inds->at(query_ind);

                        Real diff = (mts[c][start_pos + actual_ind] - mu) / sigma - query[c][query_ind];
                        dist_squared += diff * diff;
                        if (c_use_early_abandoning && dist_squared >= result_set.get_distance_lb()) {
                            points_examined += query_ind + 1;
                            point_in_entry += query_len;
                            goto start_pos_it_end_normalized;
                        }
                    }
                    points_examined += query_len;
                    point_in_entry += query_len;
                }
                result_set.insert(
                    {{subs_info.m_series_ind, subs_info.m_start_pos + start_pos, subs_info.m_length - start_pos},
                     dist_squared});
                updated = true;
            start_pos_it_end_normalized:;
                uint end_pos = start_pos + query_len;
                if (end_pos < mts_len) {
                    for (MtsNumChannelsT c : present_channels) {
                        sums[c] += mts[c][end_pos] - mts[c][start_pos];
                        sq_sums[c] += mts[c][end_pos] * mts[c][end_pos] - mts[c][start_pos] * mts[c][start_pos];
                    }
                }
                logger.increment_num_points_examined(points_examined);
                logger.increment_num_points_in_examined_entries(point_in_entry);
            }
        } else {
            for (uint start_pos = 0; start_pos < num_start_pos; ++start_pos) {
                Real dist_squared = 0;
                uint64_t points_examined = 0, point_in_entry = 0;

                for (MtsNumChannelsT c : present_channels) {
                    for (uint query_ind = 0; query_ind < query_len; ++query_ind) {
                        uint actual_ind = query_ind;
                        if constexpr (QS) actual_ind = real_query_inds->at(query_ind);

                        Real diff = mts[c][start_pos + actual_ind] - query[c][query_ind];
                        dist_squared += diff * diff;
                        if (c_use_early_abandoning && dist_squared >= result_set.get_distance_lb()) {
                            points_examined += query_ind + 1;
                            point_in_entry += query_len;
                            goto start_pos_it_end_raw;
                        }
                    }
                    points_examined += query_len;
                    point_in_entry += query_len;
                }
                result_set.insert(
                    {{subs_info.m_series_ind, subs_info.m_start_pos + start_pos, subs_info.m_length - start_pos},
                     dist_squared});
                updated = true;
            start_pos_it_end_raw:
                logger.increment_num_points_examined(points_examined);
                logger.increment_num_points_in_examined_entries(point_in_entry);
            }
        }
        logger.increment_count_col(QC::NUM_SUBS_EXAMINED, num_start_pos);

        return updated;
    }

    const bool c_normalized;
    const bool c_use_early_abandoning;
};

#endif  // SEARCH_DISTANCEMEASURE_EUCLIDEANDISTANCE_HPP
