#ifndef INDEX_LENGTHGROUPINGINDEX_HPP
#define INDEX_LENGTHGROUPINGINDEX_HPP

#include <filesystem>

#include "Index/Index.hpp"
#include "Util/RunSettings/LengthProperties.hpp"

namespace fs = std::filesystem;

/**
 * @brief Group of indexes each containing entries summarizing data about subsequences in different length ranges.
 * @tparam T The type of the entries in the index
 */
template <typename T>
class LengthGroupingIndex : public IIndex<T> {
   public:
    using FTag = typename IndexTraits<T>::FinalizedTag;

    /**
     * @brief Construct a new LengthGroupingIndex object
     * @param indexes The indexes to use for each length group
     * @param length_props The length properties to use for length groups
     */
    LengthGroupingIndex(vec<sptr<IIndex<T>>> indexes, LengthProperties length_props);

    void insert_entry_groups(vec<vec<IndexEntry<T>>> &entry_groups, EntryInserterType inserter_type) override;

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override;

    void insert(IndexEntry<T> &entry) override;

    uptr<IFinalizedIndex<FTag>> finalize() override;

    void adapt_to_dataset_groups(const vec<vec<IndexEntry<T>>> &dataset_entry_groups) override;

   private:
    vec<sptr<IIndex<T>>> m_indexes;
    LengthProperties m_length_props;

    uint get_entry_length_group(const IndexEntry<T> &entry) const;
};

#endif  // INDEX_LENGTHGROUPINGINDEX_HPP
