#include "Index/EntryMerger/DummyEntryMerger.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Traits/EntryDataSpec.hpp"
#include "Index/Traits/IndexTraits.hpp"

template <typename T>
vec<IndexEntry<T>> DummyEntryMerger<T>::merge_entries(vec<IndexEntry<T>> &&entries) {
    return entries;
}

DECLARE_ENTRY_DATA_SPECS(DummyEntryMerger)
