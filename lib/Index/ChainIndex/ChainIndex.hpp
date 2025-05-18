#ifndef INDEX_CHAININDEX_HPP
#define INDEX_CHAININDEX_HPP

#include "Index/ChainIndex/FinalizedChainIndex.hpp"
#include "Index/Entry/EntryData.hpp"
#include "Index/Index.hpp"
#include "Index/Traits/FinalizedTraits.hpp"

/**
 * @brief Set of indexes intended to be used in a chain, with the approximate indexes being used first and the exact
 * index at the end
 * @tparam T The type of the entries in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class ChainIndex : public IIndex<T> {
    using FTag = typename IndexTraits<T>::FinalizedTag;

   public:
    ChainIndex() = default;

    ChainIndex(vec<sptr<IIndex<T>>> approx_indexes, sptr<IIndex<T>> exact_index)
        : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

    uptr<IFinalizedIndex<FTag>> finalize() override {
        vec<uptr<IFinalizedIndex<FTag>>> approx_indexes;
        for (auto &index : m_approx_indexes) approx_indexes.push_back(index->finalize());
        uptr<IFinalizedIndex<FTag>> exact_index = m_exact_index->finalize();

        return uptr<IFinalizedIndex<FTag>>(
            new FinalizedChainIndex<FTag>(std::move(approx_indexes), std::move(exact_index)));
    }

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override {
        for (auto &index : m_approx_indexes) {
            // Only move the entries when doing the last insertions
            auto entries_copy = entries;
            index->insert_entries(entries_copy, inserter_type);
        }
        m_exact_index->insert_entries(entries, inserter_type);
    }

    void insert(IndexEntry<T> &entry) override {
        for (auto &index : m_approx_indexes) {
            // Only move the entry when doing the last insertions
            auto entry_copy = entry;
            index->insert(entry_copy);
        }
        m_exact_index->insert(entry);
    }

   private:
    vec<sptr<IIndex<T>>> m_approx_indexes;
    sptr<IIndex<T>> m_exact_index;
};

#endif  // INDEX_CHAININDEX_HPP
