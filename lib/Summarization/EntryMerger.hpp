#ifndef ENTRY_MERGER_HPP
#define ENTRY_MERGER_HPP

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"

/**
 * @brief Interface for merging IndexEntry objects
 * @tparam T The type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
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

/**
 * @brief Class that merges overlapping IndexEntry objects if their SAX representation is the same
 * @tparam T The type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
class SaxBasedEntryMerger : public IEntryMerger<T> {
   public:
    SaxBasedEntryMerger() = default;

    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    SaxBasedEntryMerger(SaxNumBitsT sax_num_bits) : m_sax_num_bits(sax_num_bits) {}

    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override {
        // Implement the merging logic based on SAX
        // This is a placeholder implementation
        return entries;
    }

   private:
    SaxNumBitsT m_sax_num_bits = 0;
};

#endif  // ENTRY_MERGER_HPP
