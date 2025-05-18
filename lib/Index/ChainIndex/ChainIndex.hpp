#ifndef INDEX_CHAININDEX_HPP
#define INDEX_CHAININDEX_HPP

#include "Index/ChainIndex/FinalizedChainIndex.hpp"
#include "Index/Entry/EntryData.hpp"
#include "Index/Index.hpp"
#include "Index/Traits/EntryTags.hpp"

/**
 * @brief Set of indexes intended to be used in a chain, with the approximate indexes being used first and the exact
 * index at the end
 * @tparam T The type of the entries in the index
 */
template <typename T>
class ChainIndex : public IIndex<T> {
    using FTag = typename IndexTraits<T>::FinalizedTag;

   public:
    ChainIndex() = default;

    ChainIndex(vec<sptr<IIndex<T>>> approx_indexes, sptr<IIndex<T>> exact_index);

    uptr<IFinalizedIndex<FTag>> finalize() override;

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override;

    void insert(IndexEntry<T> &entry) override;

   private:
    vec<sptr<IIndex<T>>> m_approx_indexes;
    sptr<IIndex<T>> m_exact_index;
};

#endif  // INDEX_CHAININDEX_HPP
