#ifndef INDEX_ENTRY_HPP
#define INDEX_ENTRY_HPP

#include "Index/Entry/EntryData.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

/**
 * @brief Entries of indexes
 * @tparam T The type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
struct IndexEntry {
    /** @brief Position within the dataset and length of the subsequence summarized in the entry */
    SubsequenceInfo m_subs_info;
    /** @brief Multivariate time series summary */
    vec<T> m_mts_summary;

    /**
     * @brief Equality operator
     * @param other The other object to compare to
     * @return True if the objects are equal, false otherwise
     * */
    bool operator==(const IndexEntry &other) const {
        return m_subs_info == other.m_subs_info && m_mts_summary == other.m_mts_summary;
    }

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_subs_info, m_mts_summary);
    }
};

#endif  // INDEX_ENTRY_HPP
