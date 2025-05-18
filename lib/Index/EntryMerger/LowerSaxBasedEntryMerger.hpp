#ifndef INDEX_ENTRYMERGER_LOWERSAXBASEDENTRYMERGER_HPP
#define INDEX_ENTRYMERGER_LOWERSAXBASEDENTRYMERGER_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/EntryMerger/SaxBasedEntryMerger.hpp"

/**
 * @brief Class that merges overlapping IndexEntry<T> objects if the SAX representation of their lower bounds is
 * the same
 */
class LowerSaxBasedEntryMerger : public SaxBasedEntryMerger<Envelope> {
   public:
    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    LowerSaxBasedEntryMerger(SaxNumBitsT sax_num_bits);

    vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) override;
};

#endif  // INDEX_ENTRYMERGER_LOWERSAXBASEDENTRYMERGER_HPP
