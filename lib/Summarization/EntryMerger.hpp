#ifndef ENTRY_MERGER_HPP
#define ENTRY_MERGER_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/SaxHelpers.hpp"

/** @brief Enum for IEntryMerger implementations */
enum EntryMergerType { DUMMY, SAX_BASED, LOWER_SAX_BASED };

DEFINE_ENUM_CONSTS(EntryMergerType, ENTRY_MERGER_TYPE, false,
                   (umap<str, EntryMergerType>{
                       {"sax", SAX_BASED}, {"lower_sax", LOWER_SAX_BASED}, {"none", DUMMY}}));

constexpr std::array<EntryMergerType, 2> MERGERS_W_SAX{SAX_BASED, LOWER_SAX_BASED};

/**
 * @brief Interface for merging IndexEntry objects
 * @tparam T the type of data stored in the entries
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
 * @brief Dummy implementation of IEntryMerger that does not do any merging
 * @tparam T the type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
class DummyEntryMerger : public IEntryMerger<T> {
   public:
    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override { return entries; }
};

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
    SaxBasedEntryMerger(SaxNumBitsT sax_num_bits) : m_sax_num_bits(sax_num_bits) {
        auto &RS = RunSettings::get_instance();
        m_alphabet_num_bits = RS.get_breakpoint_props().m_breakpoint_num_bits;
        m_breakpoints = &RS.get_breakpoints();

        m_isax_word_factory = [this](const vec<Real> &isax_input) {
            return iSaxWord(isax_input, *m_breakpoints, m_alphabet_num_bits,
                            vec<SaxNumBitsT>(isax_input.size(), m_sax_num_bits));
        };
    }

    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override {
        umap_hash<vec<vec<SaxSymbolT>>, vec<IndexEntry<T>>, SaxSymbolsHash> symbols_to_entries;

        for (auto &entry : entries) {
            if constexpr (std::is_same_v<T, Envelope>) {
                vec<vec<SaxSymbolT>> combined_symbols(entry.m_mts_summary.size());
                for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
                    combined_symbols[c].reserve(2 * entry.m_mts_summary[c].size());
                    auto isax_word = m_isax_word_factory(entry.m_mts_summary[c].m_lower);
                    for (SaxSegIndT s = 0; s < isax_word.size(); ++s) combined_symbols[c].push_back(isax_word[s]);
                    isax_word = m_isax_word_factory(entry.m_mts_summary[c].m_upper);
                    for (SaxSegIndT s = 0; s < isax_word.size(); ++s) combined_symbols[c].push_back(isax_word[s]);
                }
                symbols_to_entries[combined_symbols].push_back(std::move(entry));
            } else { // Paa
                auto symbols = get_entry_sax_symbols(entry, m_isax_word_factory);
                symbols_to_entries[symbols].push_back(std::move(entry));
            }
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
        std::sort(
            symbol_entries.begin(), symbol_entries.end(),
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

   private:
    SaxNumBitsT m_sax_num_bits, m_alphabet_num_bits;
    const vec<Real> *m_breakpoints;

   protected:
    iSaxWordFactory m_isax_word_factory;
};

/**
 * @brief Class that merges overlapping IndexEntry<T> objects if the SAX representation of their lower bounds is
 * the same
 * @tparam T the type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
class LowerSaxBasedEntryMerger : public SaxBasedEntryMerger<T> {
   public:
    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    LowerSaxBasedEntryMerger(SaxNumBitsT sax_num_bits) : SaxBasedEntryMerger<T>(sax_num_bits) {}

    vec<IndexEntry<T>> merge_entries(vec<IndexEntry<T>> &&entries) override {
        umap_hash<vec<vec<SaxSymbolT>>, vec<IndexEntry<T>>, SaxSymbolsHash> symbols_to_entries;

        for (auto &entry : entries) {
            auto symbols = get_entry_sax_symbols(entry, this->m_isax_word_factory);
            symbols_to_entries[symbols].push_back(std::move(entry));
        }
        entries.clear();

        for (auto [symbols, symbol_entries] : symbols_to_entries) {
            this->merge_and_add_entries(std::move(symbol_entries), entries);
        }
        return entries;
    }
};

#endif  // ENTRY_MERGER_HPP
