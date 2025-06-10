#ifndef INDEX_LENGTHGROUPINGINDEX_HPP
#define INDEX_LENGTHGROUPINGINDEX_HPP

#include <filesystem>

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/EntryData.hpp"
#include "Index/Index.hpp"
#include "Index/LengthGroupingIndex/FinalizedLengthGroupingIndex.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Index/Traits/IndexTraits.hpp"

namespace fs = std::filesystem;

/**
 * @brief Group of indexes each containing entries summarizing data about subsequences in different length ranges.
 * @tparam T The type of the entries in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class LengthGroupingIndex : public IIndex<T> {
   public:
    using FTag = typename IndexTraits<T>::FinalizedTag;

    /**
     * @brief Construct a new LengthGroupingIndex object
     * @param indexes The indexes to use for each length group
     * @param l_min Minimum query length
     * @param l_max Maximum query length
     */
    LengthGroupingIndex(vec<sptr<IIndex<T>>> indexes, uint l_min, uint l_max)
        : m_indexes(indexes), m_l_min(l_min), m_l_max(l_max) {
        assert(l_min > 0);
        assert(l_max > 0);
        assert(l_min <= l_max);
    }

    void insert_entry_groups(vec<vec<IndexEntry<T>>> &entry_groups, EntryInserterType inserter_type) override {
        OMP_PRAGMA(omp parallel for)
        for (uint l_ind = 0; l_ind < U(entry_groups.size()); ++l_ind) {
            auto &entry_group = entry_groups[l_ind];
            m_indexes[l_ind]->insert_entries(entry_group, inserter_type);
        }
    }

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override {
        for (auto &entry : entries) {
            m_indexes[get_entry_length_group(entry)]->insert(entry);
        }
    }

    void insert(IndexEntry<T> &entry) override { m_indexes[get_entry_length_group(entry)]->insert(entry); }

    uptr<IFinalizedIndex<FTag>> finalize() override {
        vec<uptr<IFinalizedIndex<FTag>>> finalized_indexes(m_indexes.size());
        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind) {
            finalized_indexes[l_ind] = m_indexes[l_ind]->finalize();
        }
        return uptr<IFinalizedIndex<FTag>>(
            new FinalizedLengthGroupingIndex<FTag>(std::move(finalized_indexes), m_l_min, m_l_max));
    }

    void adapt_to_dataset_groups(const vec<vec<IndexEntry<T>>> &dataset_entry_groups) override {
        for (uint l_ind = 0; l_ind < dataset_entry_groups.size(); ++l_ind) {
            m_indexes[l_ind]->adapt_to_dataset(dataset_entry_groups[l_ind]);
        }
    }

   private:
    vec<sptr<IIndex<T>>> m_indexes;
    uint m_l_min, m_l_max;

    inline uint get_entry_length_group(const IndexEntry<T> &entry) const {
        return RunSettings::get_instance().get_length_props().get_length_group(entry.m_subs_info.m_length);
    }
};

#endif  // INDEX_LENGTHGROUPINGINDEX_HPP
