#ifndef INDEX_ENTRY_HPP
#define INDEX_ENTRY_HPP

#include <type_traits>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Summarization/iSaxWord.hpp"

/** @brief Interface for data of IndexEntry objects */
class IEntryData {
   public:
    virtual ~IEntryData() = default;

    /**
     * @brief Get the size (number of entries) of the envelope
     * @return The size of the envelope
     * */
    virtual size_t size() const = 0;

    /**
     * @brief Resize the envelope
     * @param new_size The new size of the envelope
     * */
    virtual void resize(size_t new_size) = 0;

    /**
     * @brief Get the input for the iSAX index
     * @return The input for the iSAX index
     * */
    virtual const vec<Real> &get_isax_input() const = 0;
};

template <typename T>
concept DerivedFromEntryData = std::is_base_of_v<IEntryData, T>;

/**
 * @brief Entries of indexes
 * @tparam T The type of data stored in the entries
 */
template <typename T>
    requires DerivedFromEntryData<T>
struct IndexEntry {
    /** @brief Position within the dataset and length of the subsequence summarized in the entry */
    SubsequenceInfo m_subs_info;
    /** @brief Multivariate time series summary */
    vec<T> m_mts_summary;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_subs_info, m_mts_summary);
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
        SaxSegIndT num_segments = static_cast<SaxSegIndT>(symbols[c].size());
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

/**
 * @brief Interface for index entry generators
 * @tparam The type of entry to generate, must extend IndexEntry
 * */
template <typename T>
class IEntryGenerator {
   public:
    virtual ~IEntryGenerator() = default;

    /**
     * @brief Generate entries for a given time series, grouped by length
     * @tparam D Type of data stored in the entries
     * @param mts Multivariate time series
     * @param series_ind Index of the time series within the dataset
     * @return Entries
     */
    virtual vec<vec<IndexEntry<T>>> get_entries(const vec<vec<Real>> &mts, uint series_ind) = 0;
};

#endif  // INDEX_ENTRY_HPP
