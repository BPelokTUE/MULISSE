#include "Search/DistanceMeasure.hpp"

bool EuclideanDistance::update_result_set(IResultSet *result_set, FilePositionT file_pos, const vec<vec<float>> &query,
                                          const vec<vec<float>> &mts) {
    bool updated = false;

    unsigned num_start_pos = mts[0].size() - query[0].size() + 1;
    for (unsigned start_pos = 0; start_pos < num_start_pos; ++start_pos) {
        DistanceT dist_squared = 0;
        for (size_t c = 0; c < query.size(); ++c) {
            for (size_t i = 0; i < query[c].size(); ++i) {
                float diff = mts[c][start_pos + i] - query[c][i];
                dist_squared += diff * diff;
                if (dist_squared > result_set->get_distance_lb()) goto start_pos_it_end;
            }
        }
        result_set->insert({file_pos + start_pos, dist_squared});
        updated = true;
    start_pos_it_end:;
    }

    return updated;
};

DistanceT EuclideanDistance::min_dist_squared(const float paa, float lower, float upper) const {
    float diff = upper > paa ? upper - paa : (lower < paa ? paa - lower : 0);
    return diff * diff;
}
