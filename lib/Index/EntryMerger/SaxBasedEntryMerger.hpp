#ifndef INDEX_ENTRYMERGER_SAXBASEDENTRYMERGER_HPP
#define INDEX_ENTRYMERGER_SAXBASEDENTRYMERGER_HPP

#include "Index/EntryMerger/EntryMerger.hpp"
#include "Index/Sax/SaxSymbolsFactory.hpp"

/**
 * @brief Class that merges overlapping IndexEntry objects if their SAX representation is the same
 * @tparam T the type of data stored in the entries
 */
template <typename T>
class SaxBasedEntryMerger : public IEntryMerger<T> {
   public:
    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    SaxBasedEntryMerger(SaxNumBitsT sax_num_bits);

    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override;

   protected:
    /**
     * @brief Merge and add entries with the same symbols to the list of merged entries
     * @param symbol_entries The list of entries to merge
     * @param merged_entries The list of merged entries
     */
    void merge_and_add_entries(vec<IndexEntry<T>> &&symbol_entries, vec<IndexEntry<T>> &merged_entries);

    SaxSymbolsFactory m_sax_symbols_factory;
};

#endif  // INDEX_ENTRYMERGER_SAXBASEDENTRYMERGER_HPP
