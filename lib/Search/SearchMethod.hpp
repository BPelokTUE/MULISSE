#ifndef SEARCH_METHOD_HPP
#define SEARCH_METHOD_HPP

#include "Search/Options/SearchOptions.hpp"
#include "Search/ResultSet.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

class ISearchMethod {
   public:
    virtual ~ISearchMethod() = default;

    /**
     * @brief Search for multivariate subsequence
     *
     * @param query Multivariate subsequence to search for
     * @param search_options Search options
     * @param dataset_ifs Input file stream for the dataset
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &opts,
                                     std::ifstream &dataset_ifs) const = 0;

   protected:
    void normalize_time_series(vec<vec<float>> &mts, const vec<vec<float>> &query) const {
        for (MtsNumChannelsT c = 0; c < mts.size(); ++c) {
            if (query[c].empty()) continue;

            float sum = 0, sq_sum = 0;
            for (float value : mts[c]) {
                sum += value;
                sq_sum += value * value;
            }
            auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, mts[c].size());
            for (float &value : mts[c]) {
                value = (value - mu) / sigma;
            }
        }
    }
};

#endif  // SEARCH_METHOD_HPP
