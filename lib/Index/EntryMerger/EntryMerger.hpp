#ifndef ENTRY_MERGER_HPP
#define ENTRY_MERGER_HPP

#include "Util/Types/Vec.hpp"

template <typename T>
struct IndexEntry;

/**
 * @brief Interface for merging IndexEntry objects
 * @tparam T the type of data stored in the entries
 */
template <typename T>
class IEntryMerger {
   public:
    virtual ~IEntryMerger() = default;

    /**
     * @brief Merge entries
     * @param entries The list of entries to merge. R-value reference.
     * @return The merged entries
     */
    virtual vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) = 0;
};

#endif  // ENTRY_MERGER_HPP
