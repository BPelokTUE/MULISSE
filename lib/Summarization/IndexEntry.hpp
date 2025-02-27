#ifndef INDEX_ENTRY_HPP
#define INDEX_ENTRY_HPP

#include <type_traits>

#include <Util/typedefs.hpp>

struct EntryData {
    virtual ~EntryData() = default;

    virtual vec<float> get_isax_input() const = 0;
};

template <typename T>
concept DerivedFromEntryData = std::is_base_of_v<EntryData, T>;

template <typename T>
    requires DerivedFromEntryData<T>
struct IndexEntry {
    virtual ~IndexEntry() = default;

    /** @brief Position within the dataset of the subsequence summarized in the entry */
    SubsequencePosition subsequence_position;
    /** @brief Multivariate time series summary */
    vec<T> mts_summary;
};

/**
 * @brief Interface for index entry generators
 * @tparam The type of entry to generate, must extend IndexEntry
 * */
template <typename T>
class IEntryGenerator {
   public:
    virtual ~IEntryGenerator() = default;

    /**
     * @brief Generate entries for a given time series
     * @tparam D Type of data stored in the entries
     * @param mts Multivariate time series
     * @param series_ind Index of the time series within the dataset
     * @return Entries
     */
    virtual vec<IndexEntry<T>> get_entries(const vec<vec<float>> &mts, uint series_ind) = 0;
};

#endif  // INDEX_ENTRY_HPP
