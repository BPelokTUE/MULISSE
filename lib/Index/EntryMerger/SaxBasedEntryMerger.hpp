#ifndef INDEX_ENTRYMERGER_SAXBASEDENTRYMERGER_HPP
#define INDEX_ENTRYMERGER_SAXBASEDENTRYMERGER_HPP

#include "Index/EntryMerger/EntryMerger.hpp"
#include "Index/Sax/SaxSymbolsFactory.hpp"
#include "Util/RunSettings/RunSettings.hpp"

/**
 * @brief Class that merges overlapping IndexEntry objects if their SAX representation is the same
 * @tparam T the type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
class SaxBasedEntryMerger : public IEntryMerger<T> {
   public:
    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    SaxBasedEntryMerger(SaxNumBitsT sax_num_bits)
        : m_sax_symbols_factory(RunSettings::get_instance().get_breakpoint_props(), sax_num_bits) {}

    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override {
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

   protected:
    /**
     * @brief Merge and add entries with the same symbols to the list of merged entries
     * @param symbol_entries The list of entries to merge
     * @param merged_entries The list of merged entries
     */
    void merge_and_add_entries(vec<IndexEntry<T>> &&symbol_entries, vec<IndexEntry<T>> &merged_entries) {
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

    SaxSymbolsFactory m_sax_symbols_factory;
};

#endif  // INDEX_ENTRYMERGER_SAXBASEDENTRYMERGER_HPP
