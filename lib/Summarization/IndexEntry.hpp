#ifndef INDEX_ENTRY_HPP
#define INDEX_ENTRY_HPP

#include <type_traits>

#include <Util/typedefs.hpp>

struct IndexEntry {
    virtual ~IndexEntry() = default;

    /** @brief Position within the dataset of the subsequence summarized in the entry */
    SubsequencePosition subsequence_position;
};

// Define a concept to enforce T extends IndexEntry
template <typename T>
concept DerivedFromIndexEntry = std::is_base_of_v<IndexEntry, T>;

/**
 * @brief Interface for index entry generators
 * @tparam The type of entry to generate, must extend IndexEntry
 * */
template <typename T>
    requires DerivedFromIndexEntry<T>
class IEntryGenerator {
   public:
    virtual ~IEntryGenerator() = default;

    /**
     * @brief Generate entries for a given time series
     *
     * @param mts Multivariate time series
     * @param series_ind Index of the time series within the dataset
     * @return Entries
     */
    virtual vec<T> get_entries(const vec<vec<float>> &mts, uint series_ind) = 0;
};

#endif  // INDEX_ENTRY_HPP
