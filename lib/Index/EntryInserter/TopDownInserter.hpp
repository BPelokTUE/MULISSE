#ifndef TOP_DOWN_INSERTER_HPP
#define TOP_DOWN_INSERTER_HPP

#include "Index/EntryInserter/EntryInserter.hpp"

template <typename IndexType>
    requires ImplementsIIndex<IndexType>
class TopDownInserter : public IEntryInserter<IndexType> {
    using EntryType = typename IndexType::EntryType;

   public:
    TopDownInserter(sptr<IndexType> index) : m_index(index) {}

    void insert_entries(vec<IndexEntry<EntryType>> &entries) override {
        for (auto &entry : entries) m_index->insert(entry);
    }

   private:
    sptr<IndexType> m_index;
};

#endif  // TOP_DOWN_INSERTER_HPP
