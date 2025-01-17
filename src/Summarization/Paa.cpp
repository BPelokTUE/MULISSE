#include "Summarization/Paa.hpp"

vec<float> paa(const vec<float> &ts, unsigned segment_len) {
    unsigned num_segments = ts.size() / segment_len;
    vec<float> paa(num_segments);

    float sum;
    unsigned ind = 0, i, j;
    for (i = 0; i < num_segments; ++i) {
        sum = 0;
        for (j = 0; j < segment_len; ++j, ++ind) {
            sum += ts[ind];
        }
        paa[i] = sum / segment_len;
    }
    return paa;
}
