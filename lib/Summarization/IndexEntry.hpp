#ifndef INDEX_ENTRY_HPP
#define INDEX_ENTRY_HPP

#include <type_traits>

#include <Util/typedefs.hpp>

struct EntryData {
    virtual ~EntryData() = default;

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
    virtual vec<Real> get_isax_input() const = 0;
};

template <typename T>
concept DerivedFromEntryData = std::is_base_of_v<EntryData, T>;

template <typename T>
    requires DerivedFromEntryData<T>
struct IndexEntry {
    /** @brief Position within the dataset and length of the subsequence summarized in the entry */
    SubsequenceInfo subsequence_info;
    /** @brief Multivariate time series summary */
    vec<T> mts_summary;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(subsequence_info, mts_summary);
    }
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
    virtual vec<IndexEntry<T>> get_entries(const vec<vec<Real>> &mts, uint series_ind) = 0;
};

#endif  // INDEX_ENTRY_HPP
