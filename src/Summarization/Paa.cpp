#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Summarization/Paa.hpp"

std::vector<float> paa(const std::vector<float> &ts, unsigned segment_len) {
    unsigned m = ts.size();
    std::vector<float> paa(m / segment_len);

    float sum;
    for (unsigned i = 0; i < m; i += segment_len) {
        sum = 0;
        for (unsigned j = 0; j < segment_len; ++j) {
            sum += ts[j];
        }
        paa[i / segment_len] = sum / segment_len;
    }
    return paa;
}

/* TEST */
TEST_CASE("paa happy-flow works") {
    std::vector<float> ts = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    unsigned segment_len = 2;
    std::vector<float> expected = {1.5, 3.5, 5.5, 7.5, 9.5};
    CHECK(paa(ts, segment_len) == expected);
}
