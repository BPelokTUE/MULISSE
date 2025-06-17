#ifndef SEARCH_LENGTHGROUPINGINDEXSEARCH_HPP
#define SEARCH_LENGTHGROUPINGINDEXSEARCH_HPP

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/LengthGroupingIndex/FinalizedLengthGroupingIndex.hpp"
#include "Search/SearchMethod.hpp"

/**
 * @brief Search method for using length grouped indexes
 * @tparam S The type of the search to execute
 * @tparam D The type of the distance to use
 * @tparam QS Whether the query data points are sorted
 */
template <SearchType S, DistanceType D, bool QS = false>
class LengthGroupingIndexSearch : public ISearchMethod<S, D, QS> {
   public:
    /**
     * @brief Construct a new LengthGroupingIndexSearch object
     * @param search_methods The search methods to use for each length group
     * @param l_min Minimum query length
     * @param l_max Maximum query length
     */
    LengthGroupingIndexSearch(vec<uptr<ISearchMethod<S, D, QS>>> search_methods, uint l_min, uint l_max)
        : m_search_methods(std::move(search_methods)), m_l_min(l_min), m_l_max(l_max) {
        assert(l_min > 0);
        assert(l_max > 0);
        assert(l_min <= l_max);
    }

    inline void reset() override {
        for (auto &search_method : m_search_methods) search_method->reset();
    }

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) override {
        uint query_len;
        for (auto &channel : query) {
            if (!channel.empty()) {
                query_len = U(channel.size());
                break;
            }
        }
        uint length_group = RunSettings::get_instance().get_length_props().get_length_group(query_len);
        return m_search_methods[length_group]->search(query, opts, result_set, distance_measure, dataset_ifs,
                                                      real_query_inds);
    }

   private:
    vec<uptr<ISearchMethod<S, D, QS>>> m_search_methods;
    uint m_l_min, m_l_max;
};

#endif  // SEARCH_LENGTHGROUPINGINDEXSEARCH_HPP
