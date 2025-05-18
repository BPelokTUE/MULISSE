#include "Index/iSaxIndex/iSaxIndex.hpp"

#include "Index/EntryInserter/EntryInserter.hpp"
#include "Index/EntryInserter/ParallelInserter.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Index/Traits/IndexTraits.hpp"
#include "Index/iSaxIndex/FinalizedISaxNode.hpp"

// Template specializations of `insert_entries`, to enable interaction with `EntryInserter` implementations.

template <typename T>
    requires DerivedFromEntryData<T>
void insert_entries_impl(sptr<iSaxIndex<T>> index, vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) {
    uptr<IEntryInserter<iSaxIndex<T>>> inserter;
    switch (inserter_type) {
        case TOP_DOWN:
            inserter = std::make_unique<TopDownInserter<iSaxIndex<T>>>(index);
            break;
        case PARALLEL:
            inserter = std::make_unique<iSaxParallelInserter<T>>(index);
            break;
        default:
            throw std::invalid_argument("Invalid inserter type");
    }
    inserter->insert_entries(entries);
}

template <>
void iSaxIndex<Paa>::insert_entries(vec<IndexEntry<Paa>> &entries, EntryInserterType inserter_type) {
    insert_entries_impl<Paa>(this->shared_from_this(), entries, inserter_type);
}

template <>
void iSaxIndex<Envelope>::insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) {
    insert_entries_impl<Envelope>(this->shared_from_this(), entries, inserter_type);
}
