#include "Summarization/Paa.hpp"

std::vector<float> paa(const std::vector<float> &ts, unsigned segment_len) {
    unsigned m = ts.size();
    std::vector<float> paa(m / segment_len);

    float sum;
    for (unsigned i = 0; i < m; i += segment_len) {
        sum = 0;
        for (unsigned j = 0; j < segment_len; ++j) {
            sum += ts[i + j];
        }
        paa[i / segment_len] = sum / segment_len;
    }
    return paa;
}
