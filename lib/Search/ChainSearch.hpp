#ifndef SEARCH_INDEXSEARCH_CHAININDEXSEARCH_HPP
#define SEARCH_INDEXSEARCH_CHAININDEXSEARCH_HPP

#include "Index/ChainIndex/FinalizedChainIndex.hpp"
#include "Search/SearchMethod.hpp"

/**
 * @brief Set of search methods intended to be used in a chain, with the approximate methods being used first and the
 * exact method at the end. If the exact answer is found anywhere in the chain, the search stops.
 * @tparam S The type of the search to execute
 * @tparam D The type of the distance to use
 * @tparam QS Whether the query data points are sorted
 */
template <SearchType S, DistanceType D, bool QS = false>
class ChainSearch : public ISearchMethod<S, D, QS> {
   public:
    ChainSearch(vec<uptr<ISearchMethod<S, D, QS>>> approx_methods, uptr<ISearchMethod<S, D, QS>> exact_method)
        : m_approx_methods(std::move(approx_methods)), m_exact_method(std::move(exact_method)) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto opts_approx = opts;
        opts_approx.m_exact = false;
        for (auto &method : m_approx_methods) {
            auto search_results =
                method->search(query, opts_approx, result_set, distance_measure, dataset_ifs, real_query_inds);
            if (search_results.m_exact) return search_results;
        }
        return m_exact_method->search(query, opts, result_set, distance_measure, dataset_ifs, real_query_inds);
    }

   private:
    vec<uptr<ISearchMethod<S, D, QS>>> m_approx_methods;
    uptr<ISearchMethod<S, D, QS>> m_exact_method;
};

#endif  // SEARCH_INDEXSEARCH_CHAININDEXSEARCH_HPP
