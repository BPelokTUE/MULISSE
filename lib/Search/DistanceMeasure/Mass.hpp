#ifndef SEARCH_DISTANCEMEASURE_MASS_HPP
#define SEARCH_DISTANCEMEASURE_MASS_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Search/DistanceMeasure/DistanceMeasure.hpp"
#include "Search/SearchMethod.hpp"

template <SearchType S>
class DistanceMeasure<S, MASS> {
   public:
    /**
     * @brief Constructor
     * @param normalized Whether the time series are normalized
     */
    DistanceMeasure(bool normalized) : c_normalized(normalized) {}

    inline Real min_dist_squared(const Real paa, Real lower, Real upper) const {
        Real diff = upper < paa ? paa - upper : (lower > paa ? lower - paa : 0);
        return diff * diff;
    }

    inline bool update_result_set(ResultSet<S> &result_set, SubsequenceInfo subs_info, const vec<vec<Real>> &query,
                                  const vec<vec<Real>> &mts,
                                  std::function<bool(const SubsequencePosition &)> skip_position,
                                  const vec<uint> *real_query_inds = nullptr) const {
        // TODO: Check if query normalization can be removed in Z-normalized formula
        // TODO: Check if position skipping can or should be applied
        // TODO: Check if time series skipping can or should be applied
        auto &logger = QueryLogger::get_instance();

        bool updated = false;

        uint mts_len = 0, query_len = 0;
        MassT query_len_mt = 0.0;
        for (MtsNumChannelsT c = 0; c < mts.size(); ++c) {
            if (!query[c].empty()) {
                mts_len = U(mts[c].size());
                query_len = U(query[c].size());
                query_len_mt = static_cast<MassT>(query_len);
                break;
            }
        }
        uint num_start_pos = mts_len - query_len + 1;

        vec<Real> squared_dists(num_start_pos, 0);
        uint64_t points_examined = 0;
        for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
            if (query[c].empty()) continue;

            vec<MassT> q_channel(query_len), mts_channel(mts_len);
            for (uint i = 0; i < query_len; ++i) q_channel[i] = static_cast<MassT>(query[c][i]);

            vec<MassT> mts_sums(mts_len + 1, 0), mts_sum_sqs(mts_len + 1, 0);
            for (uint i = 1; i <= mts_len; ++i) {
                mts_channel[i - 1] = static_cast<MassT>(mts[c][i - 1]);
                mts_sums[i] = mts_sums[i - 1] + mts_channel[i - 1];
                mts_sum_sqs[i] = mts_sum_sqs[i - 1] + mts_channel[i - 1] * mts_channel[i - 1];
            }
            MassT query_sum = 0, query_sum_sq = 0;
            for (uint i = 0; i < query_len; ++i) {
                query_sum += q_channel[i];
                query_sum_sq += q_channel[i] * q_channel[i];
            }
            auto [query_mu, query_sigma] = calculate_mu_and_sigma<MassT>(query_sum, query_sum_sq, query_len);

            vec<MassT> dot_products = calculate_dot_products(q_channel, mts_channel, subs_info, c);

            if (c_normalized) {
                for (uint start_pos = 0; start_pos < num_start_pos; ++start_pos) {
                    MassT dot = dot_products[query_len - 1 + start_pos],
                          subs_sum = mts_sums[query_len + start_pos] - mts_sums[start_pos],
                          subs_sum_sq = mts_sum_sqs[query_len + start_pos] - mts_sum_sqs[start_pos];
                    auto [subs_mu, subs_sigma] = calculate_mu_and_sigma<MassT>(subs_sum, subs_sum_sq, query_len);

                    // TODO: Assuming that the query is already normalized ==> query_mu = 0, query_sigma = 1
                    Real corr =
                        R((dot - query_len_mt * query_mu * subs_mu) / (query_len_mt * query_sigma * subs_sigma));
                    squared_dists[start_pos] += std::max(R(0.0), R(2 * query_len_mt * (1 - corr)));
                }
            } else {
                for (uint start_pos = 0; start_pos < num_start_pos; ++start_pos) {
                    Real dot = R(dot_products[query_len - 1 + start_pos]);
                    squared_dists[start_pos] += std::max(
                        R(0.0),
                        R(query_sum_sq + (mts_sum_sqs[query_len + start_pos] - mts_sum_sqs[start_pos]) - 2 * dot));
                }
            }

            points_examined += query_len;
        }

        for (uint start_pos = 0; start_pos < squared_dists.size(); ++start_pos) {
            if (squared_dists[start_pos] < result_set.get_distance_lb()) {
                SubsequencePosition result_pos = {subs_info.m_position.m_series,
                                                  subs_info.m_position.m_start + start_pos};
                result_set.insert({result_pos, squared_dists[start_pos]});
                updated = true;
            }
        }

        logger.increment_num_points_examined(points_examined);
        logger.increment_num_points_in_examined_entries(points_examined);
        logger.increment_count_col(QC::NUM_SUBS_EXAMINED, num_start_pos);

        return updated;
    }

    const bool c_normalized;

   private:
    inline vec<MassT> calculate_dot_products(const vec<MassT> &q_channel, const vec<MassT> &mts_channel,
                                             SubsequenceInfo subs_info, MtsNumChannelsT channel_ind) const {
        uint mts_len = U(mts_channel.size()), query_len = U(q_channel.size());
        uint fft_size = 2 * mts_len;
        int fft_size_i = static_cast<int>(fft_size);

        FftArray query_fft(fft_size), mts_fft(fft_size), dot_prods_fft(fft_size), dot_products(fft_size);
        fftwr_plan plan;

        auto &RS = RunSettings::get_instance();

        if (RS.ffts_supported()) {
            auto &logger = QueryLogger::get_instance();

            logger.start_timer(QC::IO_TIME_S);
            mts_fft = RS.get_ffts(subs_info.m_position.m_series, channel_ind);
            logger.stop_timer(QC::IO_TIME_S);

            auto *query_fft_ptr = RS.get_query_ffts(channel_ind);
            if (!query_fft_ptr) {
                RS.calculate_query_ffts(q_channel, channel_ind);
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

        vec<MassT> dot_products_real(mts_len);
        for (uint i = 0; i < mts_len; ++i) dot_products_real[i] = dot_products[i][0] / static_cast<MassT>(fft_size);

        return dot_products_real;
    }
};

#endif  // SEARCH_DISTANCEMEASURE_MASS_HPP
