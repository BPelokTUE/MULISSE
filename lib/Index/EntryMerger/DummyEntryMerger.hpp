#ifndef INDEX_ENTRYMERGER_DUMMYENTRYMERGER_HPP
#define INDEX_ENTRYMERGER_DUMMYENTRYMERGER_HPP

#include "Index/EntryMerger/EntryMerger.hpp"

/**
 * @brief Dummy implementation of IEntryMerger that does not do any merging
 * @tparam T the type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
class DummyEntryMerger : public IEntryMerger<T> {
   public:
    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override { return entries; }
};

#endif  // INDEX_ENTRYMERGER_DUMMYENTRYMERGER_HPP
