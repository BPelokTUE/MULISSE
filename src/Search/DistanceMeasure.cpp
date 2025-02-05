#include "Search/DistanceMeasure.hpp"

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
            DistanceT mu = sums[c] / query_len, sigma = std::sqrt(std::max(sq_sums[c] / query_len - mu * mu, EPS));
            for (size_t i = 0; i < query[c].size(); ++i) {
                DistanceT diff = (mts[c][start_pos + i] - mu) / sigma - query[c][i];
                dist_squared += diff * diff;
                if (dist_squared > result_set->get_distance_lb()) {
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
