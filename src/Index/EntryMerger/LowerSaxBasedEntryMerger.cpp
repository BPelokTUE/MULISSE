#include "Index/EntryMerger/LowerSaxBasedEntryMerger.hpp"

#include "Index/Sax/SaxHelpers.hpp"

LowerSaxBasedEntryMerger::LowerSaxBasedEntryMerger(SaxNumBitsT sax_num_bits)
    : SaxBasedEntryMerger<Envelope>(sax_num_bits) {}

vec<IndexEntry<Envelope>> LowerSaxBasedEntryMerger::merge_entries(vec<IndexEntry<Envelope>> &&entries) {
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
