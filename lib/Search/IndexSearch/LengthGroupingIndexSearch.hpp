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
 * @tparam SQ Whether the query data points are sorted
 */
template <SearchType S, DistanceType D, bool SQ = false>
class LengthGroupingIndexSearch : public ISearchMethod<S, D, SQ> {
   public:
    /**
     * @brief Construct a new LengthGroupingIndexSearch object
     * @param search_methods The search methods to use for each length group
     * @param length_props The length properties to use
     */
    LengthGroupingIndexSearch(vec<uptr<ISearchMethod<S, D, SQ>>> search_methods, LengthProperties length_props)
        : m_search_methods(std::move(search_methods)), m_length_props(length_props) {
        assert(U(m_search_methods.size()) == m_length_props.m_num_l_groups);
    }

    inline void reset() override {
        for (auto &search_method : m_search_methods) search_method->reset();
    }

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, SQ> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) override {
        uint query_len;
        for (auto &channel : query) {
            if (!channel.empty()) {
                query_len = U(channel.size());
                break;
            }
        }
        uint length_group = m_length_props.get_length_group(query_len);
        return m_search_methods[length_group]->search(query, opts, result_set, distance_measure, dataset_ifs,
                                                      real_query_inds);
    }

   private:
    vec<uptr<ISearchMethod<S, D, SQ>>> m_search_methods;
    LengthProperties m_length_props;
};

#endif  // SEARCH_LENGTHGROUPINGINDEXSEARCH_HPP
