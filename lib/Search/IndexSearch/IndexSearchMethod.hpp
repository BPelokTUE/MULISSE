#ifndef SEARCH_INDEXSEARCH_INDEXSEARCHMETHOD_HPP
#define SEARCH_INDEXSEARCH_INDEXSEARCHMETHOD_HPP

#include <unordered_set>

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Search/SearchMethod.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

template <typename FTag>
struct ExtraMembersForIndexSearchMethod {};

template <>
struct ExtraMembersForIndexSearchMethod<PaaTag> {
    std::unordered_set<SubsequencePosition, SubsequencePositionHash> m_examined_positions;
};

/**
 * @brief Abstract class for index-based search methods
 * @tparam FTag The traits of the entries in the index
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 */
template <typename FTag, SearchType S, DistanceType D, bool QS = false>
    requires ValidEntryTraitsTag<FTag>
class IndexSearchMethod : public ISearchMethod<S, D, QS> {
   public:
    inline void reset() override {
        if constexpr (std::is_same_v<FTag, PaaTag>) {
            m_extras.m_examined_positions.clear();
        }
    }

    inline bool skip_position(const uint query_len, const SubsequencePosition &subs_position) override {
        if constexpr (std::is_same_v<FTag, PaaTag>) {
            if (m_extras.m_examined_positions.contains(subs_position)) return true;
            m_extras.m_examined_positions.insert(subs_position);
        }
        return false;
    }

   protected:
    /**
     * @brief Check if the given entry can be ignored/skipped during search
     * @param query_len Length of the query
     * @param series_len Length of the series
     * @param subs_info Information about the subsequence in the dataset
     * @return `true` if the entry can be skipped, `false` otherwise
     */
    inline bool skip_entry(const uint query_len, const uint series_len, const SubsequenceInfo &subs_info) {
        if constexpr (std::is_same_v<FTag, PaaTag>) {
            return subs_info.m_length < query_len;
        } else if constexpr (std::is_same_v<FTag, EnvelopeTag>) {
            return series_len - subs_info.m_position.m_start < query_len;
        }
        return false;
    }

   private:
    ExtraMembersForIndexSearchMethod<FTag> m_extras;
};

#endif  // SEARCH_INDEXSEARCH_INDEXSEARCHMETHOD_HPP
