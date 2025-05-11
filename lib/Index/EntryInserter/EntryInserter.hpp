#ifndef INDEX_ENTRYINSERTER_ENTRYINSERTER_HPP
#define INDEX_ENTRYINSERTER_ENTRYINSERTER_HPP

#include "Index/Index.hpp"

template <typename IndexType>
concept ImplementsIIndex = requires {
    typename IndexType::EntryType;
    requires std::derived_from<IndexType, IIndex<typename IndexType::EntryType>>;
};

/**
 * @brief Interface for entry inserter
 * @tparam IndexType The type of index to insert entries into
 */
template <typename IndexType>
    requires ImplementsIIndex<IndexType>
class IEntryInserter {
    using EntryType = typename IndexType::EntryType;

   public:
    virtual ~IEntryInserter() = default;

    /**
     * @brief Insert entries into the index
     * @param entries The entries to insert
     * @param inserter_type The type of inserter to use
     */
    virtual void insert_entries(vec<IndexEntry<EntryType>> &entries) = 0;
};

#endif  // INDEX_ENTRYINSERTER_ENTRYINSERTER_HPP
