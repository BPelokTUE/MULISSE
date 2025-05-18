#include "Index/ChainIndex/ChainIndex.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Util/HelperFuncs/EntryDataTemplate.hpp"

template <typename T>
ChainIndex<T>::ChainIndex(vec<sptr<IIndex<T>>> approx_indexes, sptr<IIndex<T>> exact_index)
    : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

template <typename T>
uptr<IFinalizedIndex<typename IndexTraits<T>::FinalizedTag>> ChainIndex<T>::finalize() {
    vec<uptr<IFinalizedIndex<FTag>>> approx_indexes;
    for (auto &index : m_approx_indexes) approx_indexes.push_back(index->finalize());
    uptr<IFinalizedIndex<FTag>> exact_index = m_exact_index->finalize();

    return uptr<IFinalizedIndex<FTag>>(
        new FinalizedChainIndex<FTag>(std::move(approx_indexes), std::move(exact_index)));
}

template <typename T>
void ChainIndex<T>::insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) {
    for (auto &index : m_approx_indexes) {
        // Only move the entries when doing the last insertions
        auto entries_copy = entries;
        index->insert_entries(entries_copy, inserter_type);
    }
    m_exact_index->insert_entries(entries, inserter_type);
}

template <typename T>
void ChainIndex<T>::insert(IndexEntry<T> &entry) {
    for (auto &index : m_approx_indexes) {
        // Only move the entry when doing the last insertions
        auto entry_copy = entry;
        index->insert(entry_copy);
    }
    m_exact_index->insert(entry);
}

DECLARE_ENTRY_DATA_SPECS(ChainIndex)
