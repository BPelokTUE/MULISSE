#ifndef INDEX_SAX_SAXHELPERS_HPP
#define INDEX_SAX_SAXHELPERS_HPP

#include <boost/functional/hash.hpp>
#include <functional>

#include "Index/Entry/EntryData.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Sax/iSaxWord.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Hash function for SaxSymbolT */
struct SaxSymbolsHash {
    size_t operator()(const vec<vec<SaxSymbolT>> &symbols) const {
        size_t seed = 0;
        for (auto &channel : symbols) {
            for (SaxSymbolT symbol : channel) {
                boost::hash_combine(seed, symbol);
            }
        }
        return seed;
    }
};

using iSaxWordFactory = std::function<iSaxWord(const vec<Real> &)>;

/**
 * @brief Calculate iSAX words for the given entry
 * @tparam T The type of data stored in the entries
 * @param entry The entry to calculate iSAX words for
 * @param isax_word_factory The function to use for calculating the iSAX words
 * @return The iSAX words for the entry
 */
template <typename T>
    requires DerivedFromEntryData<T>
inline vec<iSaxWord> get_entry_isax(const IndexEntry<T> &entry, iSaxWordFactory isax_word_factory) {
    vec<iSaxWord> isax_words;
    isax_words.reserve(entry.m_mts_summary.size());
    for (auto channel_summary : entry.m_mts_summary)
        isax_words.push_back(isax_word_factory(channel_summary.get_isax_input()));
    return isax_words;
}

/**
 * @brief Calculate SAX symbols for the given entry
 * @tparam T The type of data stored in the entries
 * @param entry The entry to calculate SAX symbols for
 * @param isax_word_factory The function to use for calculating the iSAX words
 * @return The SAX symbols for the entry
 */
template <typename T>
    requires DerivedFromEntryData<T>
inline vec<vec<SaxSymbolT>> get_entry_sax_symbols(const IndexEntry<T> &entry, iSaxWordFactory isax_word_factory) {
    vec<vec<SaxSymbolT>> symbols(entry.m_mts_summary.size());
    for (MtsNumChannelsT c = 0; c < symbols.size(); ++c) {
        auto isax_word = isax_word_factory(entry.m_mts_summary[c].get_isax_input());
        SaxSegIndT num_segments = static_cast<SaxSegIndT>(isax_word.size());
        symbols[c].resize(num_segments);
        for (SaxSegIndT s = 0; s < num_segments; ++s) symbols[c][s] = isax_word[s];
    }
    return symbols;
}

/**
 * @brief Calculate SAX symbols and iSAX words for the given entry
 * @tparam T The type of data stored in the entries
 * @param entry The entry to calculate SAX symbols and iSAX words for
 * @param isax_word_factory The function to use for calculating the iSAX words
 * @return The SAX symbols and iSAX words for the entry
 */
template <typename T>
    requires DerivedFromEntryData<T>
inline std::pair<vec<vec<SaxSymbolT>>, vec<iSaxWord>> get_entry_sax_symbols_and_isax(
    const IndexEntry<T> &entry, iSaxWordFactory isax_word_factory) {
    vec<vec<SaxSymbolT>> symbols(entry.m_mts_summary.size());
    vec<iSaxWord> isax_words(entry.m_mts_summary.size());
    for (MtsNumChannelsT c = 0; c < isax_words.size(); ++c) {
        isax_words[c] = isax_word_factory(entry.m_mts_summary[c].get_isax_input());
        SaxSegIndT num_segments = static_cast<SaxSegIndT>(isax_words[c].size());
        symbols[c].resize(num_segments);
        for (SaxSegIndT s = 0; s < num_segments; ++s) symbols[c][s] = isax_words[c][s];
    }
    return {symbols, isax_words};
}

#endif  // INDEX_SAX_SAXHELPERS_HPP
