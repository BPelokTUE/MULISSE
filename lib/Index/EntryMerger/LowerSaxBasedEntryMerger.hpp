#ifndef INDEX_ENTRYMERGER_LOWERSAXBASEDENTRYMERGER_HPP
#define INDEX_ENTRYMERGER_LOWERSAXBASEDENTRYMERGER_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/EntryMerger/SaxBasedEntryMerger.hpp"
#include "Index/Sax/SaxHelpers.hpp"

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
    LowerSaxBasedEntryMerger(SaxNumBitsT sax_num_bits) : SaxBasedEntryMerger<Envelope>(sax_num_bits) {}

    vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) override {
        umap_hash<vec<vec<SaxSymbolT>>, vec<IndexEntry<Envelope>>, SaxSymbolsHash> symbols_to_entries;

        for (auto &entry : entries) {
            vec<vec<SaxSymbolT>> symbols;
            symbols.reserve(entry.m_mts_summary.size());
            for (auto &summary : entry.m_mts_summary)
                symbols.push_back(this->m_sax_symbols_factory.get_symbols(summary.m_lower));
            symbols_to_entries[symbols].push_back(std::move(entry));
        }
        entries.clear();

        for (auto [symbols, symbol_entries] : symbols_to_entries) {
            this->merge_and_add_entries(std::move(symbol_entries), entries);
        }
        return entries;
    }
};

#endif  // INDEX_ENTRYMERGER_LOWERSAXBASEDENTRYMERGER_HPP
