#ifndef DISTANCE_MEASURE_HPP
#define DISTANCE_MEASURE_HPP

#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/FftArray.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"
#include "Search/Options/DistanceType.hpp"
#include "Search/Options/SearchType.hpp"
#include "Search/ResultSet.hpp"

/**
 * @brief Interface for distance measures
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 * */
template <SearchType S, DistanceType D, bool QS = false>
class DistanceMeasure {
   public:
    /** @brief Get the type of the distance measure */
    DistanceType get_type() const { return D; };

    /**
     * @brief Update the result set with the subsequences from the time series
     * @param result_set Result set to update
     * @param subs_info Information about the subsequence in the dataset
     * @param query Query time series
     * @param mts Time series to update the result set with
     * @param real_query_inds Real indices of the query points (to support sorted queries for early abandoning)
     * @return true if the result set was updated, false otherwise
     */
    bool update_result_set(ResultSet<S> &result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                           const vec<vec<Real>> &mts, const vec<uint> *real_query_inds = nullptr) const;
};

// ED

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

    /**
     * @brief Calculate the minimum distance squared between a PAA value and a segment
     * @param paa PAA value
     * @param lower Lower bound of the segment
     * @param upper Upper bound of the segment
     * @return Distance squared
     */
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
                query_len = static_cast<uint>(query[c].size());
                mts_len = static_cast<uint>(mts[c].size());
                assert(mts_len >= query_len);
                num_start_pos = static_cast<uint>(mts_len - query_len + 1);
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
                        uint real_ind = query_ind;
                        if constexpr (QS) real_ind = real_query_inds->at(query_ind);

                        Real diff = (mts[c][start_pos + real_ind] - mu) / sigma - query[c][query_ind];
                        dist_squared += diff * diff;
                        if (c_use_early_abandoning && dist_squared >= result_set.get_distance_lb()) {
                            points_examined += query_ind + 1;
                            point_in_entry += query_len;
                            goto start_pos_it_end;
                        }
                    }
                    points_examined += query_len;
                    point_in_entry += query_len;
                }
                result_set.insert(
                    {{subs_info.m_series_ind, subs_info.m_start_pos + start_pos, subs_info.m_length - start_pos},
                     dist_squared});
                updated = true;
            start_pos_it_end:;
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
            throw std::runtime_error("Non-normalized Euclidean distance not implemented yet");
        }

        return updated;
    }

    const bool c_normalized;
    const bool c_use_early_abandoning;
};

template <SearchType S>
class DistanceMeasure<S, MASS> {
   public:
    /**
     * @brief Constructor
     * @param normalized Whether the time series are normalized
     */
    DistanceMeasure(bool normalized) : c_normalized(normalized) {}

    /**
     * @brief Calculate the minimum distance squared between a PAA value and a segment
     * @param paa PAA value
     * @param lower Lower bound of the segment
     * @param upper Upper bound of the segment
     * @return Distance squared
     */
    inline Real min_dist_squared(const Real paa, Real lower, Real upper) const {
        Real diff = upper < paa ? paa - upper : (lower > paa ? lower - paa : 0);
        return diff * diff;
    }

    inline bool update_result_set(ResultSet<S> &result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                                  const vec<vec<Real>> &mts, const vec<uint> *real_query_inds = nullptr) const {
        bool updated = false;

        uint mts_len = 0, query_len = 0;
        Real query_len_r = 0.0;
        for (MtsNumChannelsT c = 0; c < mts.size(); ++c) {
            if (!query[c].empty()) {
                mts_len = static_cast<uint>(mts[c].size());
                query_len = static_cast<uint>(query[c].size());
                query_len_r = R(query_len);
                break;
            }
        }

        vec<Real> squared_dists(mts_len - query_len + 1, 0);
        for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
            if (query[c].empty()) continue;

            vec<Real> q_channel(query_len), mts_channel(mts_len);
            for (uint i = 0; i < query_len; ++i) q_channel[i] = query[c][i];

            vec<Real> mts_sums(mts_len + 1, 0), mts_sum_sqs(mts_len + 1, 0);
            for (uint i = 1; i <= mts_len; ++i) {
                mts_channel[i - 1] = mts[c][i - 1];
                mts_sums[i] = mts_sums[i - 1] + mts_channel[i - 1];
                mts_sum_sqs[i] = mts_sum_sqs[i - 1] + mts_channel[i - 1] * mts_channel[i - 1];
            }
            Real query_sum = 0, query_sum_sq = 0;
            for (uint i = 0; i < query_len; ++i) {
                query_sum += q_channel[i];
                query_sum_sq += q_channel[i] * q_channel[i];
            }
            auto [query_mu, query_sigma] = calculate_mu_and_sigma(query_sum, query_sum_sq, query_len);

            vec<Real> dot_products = calculate_dot_products(q_channel, mts_channel, subs_info, c);

            if (c_normalized) {
                for (uint start_pos = 0; start_pos < mts_len - query_len + 1; ++start_pos) {
                    Real dot = dot_products[query_len - 1 + start_pos],
                         subs_sum = mts_sums[query_len + start_pos] - mts_sums[start_pos],
                         subs_sum_sq = mts_sum_sqs[query_len + start_pos] - mts_sum_sqs[start_pos];
                    auto [subs_mu, subs_sigma] = calculate_mu_and_sigma(subs_sum, subs_sum_sq, query_len);

                    // TODO: Assuming that the query is already normalized ==> query_mu = 0, query_sigma = 1
                    Real corr = (dot - query_len_r * query_mu * subs_mu) / (query_len_r * query_sigma * subs_sigma);
                    squared_dists[start_pos] += std::max(R(0.0), 2 * query_len_r * (1 - corr));
                }
            } else {
                for (uint start_pos = 0; start_pos < mts_len - query_len + 1; ++start_pos) {
                    Real dot = dot_products[query_len - 1 + start_pos];
                    squared_dists[start_pos] += std::max(
                        R(0.0), query_sum_sq + (mts_sum_sqs[query_len + start_pos] - mts_sum_sqs[start_pos]) + dot);
                }
            }
        }

        for (uint start_pos = 0; start_pos < squared_dists.size(); ++start_pos) {
            if (squared_dists[start_pos] < result_set.get_distance_lb()) {
                SubsequenceInfo result_pos = {subs_info.m_series_ind, subs_info.m_start_pos + start_pos};
                result_set.insert({result_pos, squared_dists[start_pos]});
                updated = true;
            }
        }

        return updated;
    }

    const bool c_normalized;

   private:
    inline vec<Real> calculate_dot_products(const vec<Real> &q_channel, const vec<Real> &mts_channel,
                                            SubsequenceInfo subs_info, MtsNumChannelsT channel_ind) const {
        uint mts_len = static_cast<uint>(mts_channel.size()), query_len = static_cast<uint>(q_channel.size());
        uint fft_size = 2 * mts_len;
        int fft_size_i = static_cast<int>(fft_size);

        FftArray query_fft(fft_size), mts_fft(fft_size), dot_prods_fft(fft_size), dot_products(fft_size);
        fftwr_plan plan;

        auto &RS = RunSettings::get_instance();

        if (RS.ffts_supported()) {
            auto &logger = QueryLogger::get_instance();

            logger.start_timer(QC::IO_TIME_S);
            mts_fft = RS.get_ffts(subs_info, channel_ind, mts_len);
            logger.stop_timer(QC::IO_TIME_S);

            auto *query_fft_ptr = RS.get_query_ffts(channel_ind);
            if (!query_fft_ptr) {
                RS.calculate_query_ffts(q_channel, channel_ind, mts_len);
                query_fft_ptr = RS.get_query_ffts(channel_ind);
            }
            query_fft = *query_fft_ptr;
        } else {
            FftArray mts_complex(fft_size);
            for (uint i = 0; i < mts_len; ++i) mts_complex[i][0] = mts_channel[i];
            plan = fftwr_plan_dft_1d(fft_size_i, mts_complex.data(), mts_fft.data(), FFTW_FORWARD, FFTW_ESTIMATE);
            fftwr_execute(plan);
            fftwr_destroy_plan(plan);

            FftArray q_complex(fft_size);
            for (uint i = 0; i < query_len; ++i) q_complex[i][0] = q_channel[query_len - 1 - i];
            plan = fftwr_plan_dft_1d(fft_size_i, q_complex.data(), query_fft.data(), FFTW_FORWARD, FFTW_ESTIMATE);
            fftwr_execute(plan);
            fftwr_destroy_plan(plan);
        }

        for (uint i = 0; i < fft_size; ++i) {
            dot_prods_fft[i][0] = query_fft[i][0] * mts_fft[i][0] - query_fft[i][1] * mts_fft[i][1];
            dot_prods_fft[i][1] = query_fft[i][0] * mts_fft[i][1] + query_fft[i][1] * mts_fft[i][0];
        }

        plan = fftwr_plan_dft_1d(fft_size_i, dot_prods_fft.data(), dot_products.data(), FFTW_BACKWARD, FFTW_ESTIMATE);
        fftwr_execute(plan);
        fftwr_destroy_plan(plan);

        vec<Real> dot_products_real(mts_len);
        for (uint i = 0; i < mts_len; ++i) dot_products_real[i] = dot_products[i][0] / R(fft_size);

        return dot_products_real;
    }
};

#endif  // DISTANCE_MEASURE_HPP
