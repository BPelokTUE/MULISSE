#ifndef INDEX_ENTRYINSERTER_PARALLELINSERTER_HPP
#define INDEX_ENTRYINSERTER_PARALLELINSERTER_HPP

#include "Index/EntryInserter/EntryInserter.hpp"
#include "Index/iSaxIndex/iSaxIndex.hpp"

template <typename T>
class iSaxParallelInserter : public IEntryInserter<iSaxIndex<T>> {
   public:
    iSaxParallelInserter(sptr<iSaxIndex<T>> index);

    void insert_entries(vec<IndexEntry<T>> &entries) override;

   private:
    sptr<iSaxIndex<T>> m_index;
};

#endif  // INDEX_ENTRYINSERTER_PARALLELINSERTER_HPP
