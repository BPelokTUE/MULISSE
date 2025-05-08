#include <algorithm>

#include "Summarization/EnvelopeEntryMerger.hpp"

SaxBasedEnvelopeEntryMerger::SaxBasedEnvelopeEntryMerger(SaxNumBitsT sax_num_bits) : m_sax_num_bits(sax_num_bits) {
    auto &RS = RunSettings::get_instance();
    m_alphabet_num_bits = RS.get_breakpoint_props().m_breakpoint_num_bits;
    m_breakpoints = &RS.get_breakpoints();

    m_isax_word_factory = [this](const vec<Real> &isax_input) {
        return iSaxWord(isax_input, *m_breakpoints, m_alphabet_num_bits,
                        vec<SaxNumBitsT>(isax_input.size(), m_sax_num_bits));
    };
}

vec<IndexEntry<Envelope>> SaxBasedEnvelopeEntryMerger::merge_entries(vec<IndexEntry<Envelope>> &&entries) {
    umap_hash<vec<vec<SaxSymbolT>>, vec<IndexEntry<Envelope>>, SaxSymbolsHash> symbols_to_entries;

    for (auto &entry : entries) {
        auto symbols = get_entry_sax_symbols(entry, m_isax_word_factory);
        symbols_to_entries[symbols].push_back(std::move(entry));
    }
    entries.clear();

    for (auto [symbols, symbol_entries] : symbols_to_entries) {
        std::sort(
            symbol_entries.begin(), symbol_entries.end(),
            [](const IndexEntry<Envelope> &a, const IndexEntry<Envelope> &b) { return a.m_subs_info < b.m_subs_info; });

        // Merge the entries with the same symbols
        uint rightmost = 0;
        vec<IndexEntry<Envelope>> merged_entries;
        for (auto &entry : symbol_entries) {
            uint entry_rightmost = entry.m_subs_info.m_start_pos + entry.m_subs_info.m_length - 1;
            if (merged_entries.empty() || entry.m_subs_info.m_start_pos > rightmost) {
                merged_entries.push_back(std::move(entry));
                rightmost = entry_rightmost;
            } else {
                auto &last_entry = merged_entries.back();
                if (entry_rightmost > rightmost) {
                    rightmost = entry_rightmost;
                    last_entry.m_subs_info.m_length =
                        rightmost - last_entry.m_subs_info.m_start_pos + 1;
                }
                for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c)
                    last_entry.m_mts_summary[c].merge(entry.m_mts_summary[c]);
            }
        }
        entries.insert(entries.end(), std::make_move_iterator(merged_entries.begin()),
                       std::make_move_iterator(merged_entries.end()));
    }
    return entries;
}
