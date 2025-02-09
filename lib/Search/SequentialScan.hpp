#ifndef SEQUENTIAL_SCAN_HPP
#define SEQUENTIAL_SCAN_HPP

#include "Search/SearchMethod.hpp"
#include "Search/DistanceMeasure.hpp"
#include "Search/Options/SearchOptions.hpp"

class SequentialScan : public ISearchMethod {
   public:
    SequentialScan() = default;

    vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &opts,
                             std::ifstream &dataset_ifs) const override;
};

#endif  // SEQUENTIAL_SCAN_HPP
