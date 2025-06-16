#ifndef MODULES_INDEXING_GETENTRYMERGER_HPP
#define MODULES_INDEXING_GETENTRYMERGER_HPP

#include "Index/EntryMerger/DummyEntryMerger.hpp"
#include "Index/EntryMerger/LowerSaxBasedEntryMerger.hpp"
#include "Index/EntryMerger/SaxBasedEntryMerger.hpp"
#include "Index/IndexOptions.hpp"

template <typename T>
    requires DerivedFromEntryData<T>
uptr<IEntryMerger<T>> get_entry_merger(const IndexOptions &opts);

#endif  // MODULES_INDEXING_GETENTRYMERGER_HPP
