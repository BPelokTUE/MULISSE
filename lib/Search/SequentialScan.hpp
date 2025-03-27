#ifndef SEQUENTIAL_SCAN_HPP
#define SEQUENTIAL_SCAN_HPP

#include "Search/SearchMethod.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Util/typedefs.hpp"

class SequentialScan : public ISearchMethod {
   public:
    SequentialScan() = default;

    vec<SearchResult> search(const vec<vec<Real>> &query, const SearchOptions &opts,
                             std::ifstream &dataset_ifs) const override;
};

#endif  // SEQUENTIAL_SCAN_HPP
