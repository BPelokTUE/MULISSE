#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

enum SearchType { KNN, R_RANGE };

struct KnnParameters {
    unsigned k;
};

struct RRangeParameters {
    float r;
};

union SearchParameters {
    KnnParameters knn_parameters;
    RRangeParameters r_range_parameters;
};

struct SearchOptions {
    SearchType search_type;
    SearchParameters search_parameters;
    bool exact = true, normalized = true;
};

#endif  // SEARCH_OPTIONS_HPP
