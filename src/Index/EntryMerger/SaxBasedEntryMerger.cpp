#include "Index/EntryMerger/SaxBasedEntryMerger.hpp"

#include <algorithm>

#include "Index/Sax/SaxHelpers.hpp"
#include "Index/Traits/EntryDataSpec.hpp"
#include "Index/Traits/IndexTraits.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <typename T>
SaxBasedEntryMerger<T>::SaxBasedEntryMerger(SaxNumBitsT sax_num_bits)
    : m_sax_symbols_factory(RunSettings::get_instance().get_breakpoint_props(), sax_num_bits) {}

template <typename T>
vec<IndexEntry<T>> SaxBasedEntryMerger<T>::merge_entries(vec<IndexEntry<T>> &&entries) {
    umap_hash<vec<vec<SaxSymbolT>>, vec<IndexEntry<T>>, SaxSymbolsHash> symbols_to_entries;

    for (auto &entry : entries) {
        vec<vec<SaxSymbolT>> symbols(entry.m_mts_summary.size());
        for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
            if constexpr (std::is_same_v<T, Envelope>) {
                symbols[c] = m_sax_symbols_factory.get_symbols(entry.m_mts_summary[c].m_lower);
                auto upper_symbols = m_sax_symbols_factory.get_symbols(entry.m_mts_summary[c].m_upper);
                symbols[c].insert(symbols[c].end(), upper_symbols.begin(), upper_symbols.end());
            } else {  // Paa
                symbols[c] = m_sax_symbols_factory.get_symbols(entry.m_mts_summary[c].m_paa_values);
            }
        }
        symbols_to_entries[symbols].push_back(std::move(entry));
    }
    entries.clear();

    for (auto [symbols, symbol_entries] : symbols_to_entries) {
        merge_and_add_entries(std::move(symbol_entries), entries);
    }
    return entries;
}

template <typename T>
void SaxBasedEntryMerger<T>::merge_and_add_entries(vec<IndexEntry<T>> &&symbol_entries,
                                                   vec<IndexEntry<T>> &merged_entries) {
    std::sort(symbol_entries.begin(), symbol_entries.end(),
              [](const IndexEntry<T> &a, const IndexEntry<T> &b) { return a.m_subs_info < b.m_subs_info; });

    // Merge the entries with the same symbols
    uint rightmost = 0;
    vec<IndexEntry<T>> symbol_merged_entries;
    for (auto &entry : symbol_entries) {
        uint entry_rightmost = entry.m_subs_info.m_start_pos + entry.m_subs_info.m_length - 1;
        if (symbol_merged_entries.empty() || entry.m_subs_info.m_start_pos > rightmost + 1) {
            symbol_merged_entries.push_back(std::move(entry));
            rightmost = entry_rightmost;
        } else {
            auto &last_entry = symbol_merged_entries.back();
            if (entry_rightmost > rightmost) {
                rightmost = entry_rightmost;
                last_entry.m_subs_info.m_length = rightmost - last_entry.m_subs_info.m_start_pos + 1;
            }
            for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c)
                last_entry.m_mts_summary[c].merge(entry.m_mts_summary[c]);
        }
    }
    merged_entries.insert(merged_entries.end(), std::make_move_iterator(symbol_merged_entries.begin()),
                          std::make_move_iterator(symbol_merged_entries.end()));
}

DECLARE_ENTRY_DATA_SPECS(SaxBasedEntryMerger)
