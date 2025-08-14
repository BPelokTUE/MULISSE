#ifndef INDEX_ENTRYGENERATOR_ENTRYGENERATOR_HPP
#define INDEX_ENTRYGENERATOR_ENTRYGENERATOR_HPP

#include "Index/Entry/EntryData.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

template <typename T>
class IndexEntry;

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

#endif  // INDEX_ENTRYGENERATOR_ENTRYGENERATOR_HPP
