#include <fftw3.h>

#include <iostream>

#include "Search/DistanceMeasure.hpp"
#include "Util/FftArray.hpp"
#include "Util/RunSettings.hpp"

bool EuclideanDistance::update_result_set(IResultSet *result_set, FilePositionT file_pos, const vec<vec<float>> &query,
                                          const vec<vec<float>> &mts) {
    bool updated = false;
    int num_start_pos, query_len;

    vec<DistanceT> sums(query.size()), sq_sums(query.size());
    for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
        if (!(query[c].empty())) {
            query_len = query[c].size();
            num_start_pos = mts[c].size() - query_len + 1;

            for (size_t i = 0; i < query[c].size(); ++i) {
                sums[c] += mts[c][i];
                sq_sums[c] += mts[c][i] * mts[c][i];
            }
        }
    }

    for (int start_pos = 0; start_pos < num_start_pos; ++start_pos) {
        DistanceT dist_squared = 0;
        for (size_t c = 0; c < query.size(); ++c) {
            auto [mu, sigma] = calculate_mu_and_sigma(sums[c], sq_sums[c], query_len);
            for (size_t i = 0; i < query[c].size(); ++i) {
                DistanceT diff = (mts[c][start_pos + i] - mu) / sigma - query[c][i];
                dist_squared += diff * diff;
                if (dist_squared >= result_set->get_distance_lb()) {
                    goto start_pos_it_end;
                }
            }
        }
        result_set->insert({file_pos + start_pos, dist_squared});
        updated = true;
    start_pos_it_end:;
        int end_pos = start_pos + query[0].size();
        if (end_pos < mts[0].size()) {
            for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
                if (query[c].empty()) continue;

                sums[c] += mts[c][end_pos] - mts[c][start_pos];
                sq_sums[c] += mts[c][end_pos] * mts[c][end_pos] - mts[c][start_pos] * mts[c][start_pos];
            }
        }
    }

    return updated;
};

DistanceT EuclideanDistance::min_dist_squared(const float paa, float lower, float upper) const {
    DistanceT diff = upper < paa ? paa - upper : (lower > paa ? lower - paa : 0);
    return diff * diff;
}

DistanceType EuclideanDistance::get_type() const { return ED; }

// MASS

EuclideanDistanceWMass::EuclideanDistanceWMass(bool normalized) : m_normalized(normalized) {}

bool printed = false;

vec<DistanceT> EuclideanDistanceWMass::calculate_dot_products(const vec<DistanceT> &q_channel,
                                                              const vec<DistanceT> &mts_channel, FilePositionT file_pos,
                                                              uint channel_ind) const {
    uint mts_len = mts_channel.size(), query_len = q_channel.size();

    FftArray query_fft(2 * mts_len), mts_fft(2 * mts_len), dot_prods_fft(2 * mts_len), dot_products(2 * mts_len);
    fftw_plan plan;

    auto &run_settings = RunSettings::get_instance();

    if (run_settings.ffts_supported()) {
        mts_fft = run_settings.get_ffts(file_pos, channel_ind, mts_len);

        auto *query_fft_ptr = run_settings.get_query_ffts(channel_ind);
        if (!query_fft_ptr) {
            run_settings.calculate_query_ffts(q_channel, channel_ind, mts_len);
            query_fft_ptr = run_settings.get_query_ffts(channel_ind);
        }
        query_fft = *query_fft_ptr;
    } else {
        FftArray mts_complex(2 * mts_len);
        for (uint i = 0; i < mts_len; ++i) mts_complex[i][0] = mts_channel[i];
        plan = fftw_plan_dft_1d(2 * mts_len, mts_complex.data(), mts_fft.data(), FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);

        FftArray q_complex(2 * mts_len);
        for (uint i = 0; i < query_len; ++i) q_complex[i][0] = q_channel[query_len - 1 - i];
        plan = fftw_plan_dft_1d(2 * mts_len, q_complex.data(), query_fft.data(), FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
    }

    for (uint i = 0; i < 2 * mts_len; ++i) {
        dot_prods_fft[i][0] = query_fft[i][0] * mts_fft[i][0] - query_fft[i][1] * mts_fft[i][1];
        dot_prods_fft[i][1] = query_fft[i][0] * mts_fft[i][1] + query_fft[i][1] * mts_fft[i][0];
    }

    plan = fftw_plan_dft_1d(2 * mts_len, dot_prods_fft.data(), dot_products.data(), FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);

    vec<DistanceT> dot_products_real(mts_len);
    for (uint i = 0; i < mts_len; ++i) dot_products_real[i] = dot_products[i][0] / (2 * mts_len);

    return dot_products_real;
}

// TODO: figure out where double is actually needed
bool EuclideanDistanceWMass::update_result_set(IResultSet *result_set, FilePositionT file_pos,
                                               const vec<vec<float>> &query, const vec<vec<float>> &mts) {
    bool updated = false;

    uint mts_len = mts[0].size(), query_len = 0;
    for (auto channel : query) {
        if (!channel.empty()) {
            query_len = channel.size();
            break;
        }
    }

    vec<DistanceT> squared_dists(mts_len - query_len + 1, 0);
    for (MtsNumChannelsT c = 0; c < query.size(); ++c) {
        if (query.empty()) continue;

        vec<DistanceT> q_channel(query_len), mts_channel(mts_len);
        for (uint i = 0; i < query_len; ++i) q_channel[i] = query[c][i];

        vec<DistanceT> mts_sums(mts_len + 1, 0), mts_sum_sqs(mts_len + 1, 0);
        for (uint i = 1; i <= mts_len; ++i) {
            mts_channel[i - 1] = mts[c][i - 1];
            mts_sums[i] = mts_sums[i - 1] + mts_channel[i - 1];
            mts_sum_sqs[i] = mts_sum_sqs[i - 1] + mts_channel[i - 1] * mts_channel[i - 1];
        }
        DistanceT query_sum = 0, query_sum_sq = 0;
        for (uint i = 0; i < query_len; ++i) {
            query_sum += q_channel[i];
            query_sum_sq += q_channel[i] * q_channel[i];
        }
        auto [query_mu, query_sigma] = calculate_mu_and_sigma(query_sum, query_sum_sq, query_len);

        vec<DistanceT> dot_products = calculate_dot_products(q_channel, mts_channel, file_pos, c);

        if (m_normalized) {
            for (uint start_pos = 0; start_pos < mts_len - query_len + 1; ++start_pos) {
                DistanceT dot = dot_products[query_len - 1 + start_pos],
                          subs_sum = mts_sums[query_len + start_pos] - mts_sums[start_pos],
                          subs_sum_sq = mts_sum_sqs[query_len + start_pos] - mts_sum_sqs[start_pos];
                auto [subs_mu, subs_sigma] = calculate_mu_and_sigma(subs_sum, subs_sum_sq, query_len);

                // TODO: Assuming that the query is already normalized ==> query_mu = 0, query_sigma = 1
                DistanceT corr = (dot - query_len * query_mu * subs_mu) / (query_len * query_sigma * subs_sigma);
                squared_dists[start_pos] += 2 * query_len * (1 - corr);
            }
        } else {
            for (uint start_pos = 0; start_pos < mts_len - query_len + 1; ++start_pos) {
                DistanceT dot = dot_products[query_len - 1 + start_pos];
                squared_dists[start_pos] +=
                    query_sum_sq + (mts_sum_sqs[query_len + start_pos] - mts_sum_sqs[start_pos]) + dot;
            }
        }
    }

    for (uint start_pos = 0; start_pos < squared_dists.size(); ++start_pos) {
        if (squared_dists[start_pos] < result_set->get_distance_lb()) {
            result_set->insert({file_pos + start_pos, squared_dists[start_pos]});
            updated = true;
        }
    }

    return updated;
}

DistanceType EuclideanDistanceWMass::get_type() const { return MASS; }
