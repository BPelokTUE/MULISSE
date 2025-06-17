#ifndef INDEX_INDEX_HPP
#define INDEX_INDEX_HPP

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>

#include "Enums/DistanceType.hpp"
#include "Enums/EntryInserterType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/EntryData.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Index/Traits/IndexTraits.hpp"
#include "Util/Types/Pointers.hpp"

template <typename T>
class IEntryGenerator;

template <typename T>
class IEntryMerger;

template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class IFinalizedIndex;

template <typename T>
class IndexEntry;

/**
 * @brief Interface for indexes
 * @tparam The type of entry to insert into the index
 * */
template <typename T>
class IIndex {
   public:
    using EntryType = T;
    using FTag = typename IndexTraits<T>::FinalizedTag;

    virtual ~IIndex() = default;

    /**
     * @brief Adapt the index to the dataset entries
     * @param dataset_entries The entries to adapt to
     */
    virtual void adapt_to_dataset(const vec<IndexEntry<T>> &dataset_entries) {}

    /**
     * @brief Adapt the index to the dataset entry groups
     * @param dataset_entries The entry groups to adapt to
     */
    virtual void adapt_to_dataset_groups(const vec<vec<IndexEntry<T>>> &dataset_entry_groups) {
        if (dataset_entry_groups.size() != 1) {
            throw std::runtime_error("This index does not support multiple entry groups");
        }
        adapt_to_dataset(dataset_entry_groups[0]);
    }

    /**
     * @brief Finalize the index
     *
     * Creates a finalized index, that can no longer be inserted into, but can be used for searching.
     *
     * @return A unique pointer to the finalized index
     */
    virtual uptr<IFinalizedIndex<FTag>> finalize() = 0;

    /**
     * @brief Insert entry groups into the index
     *
     * This function facilitates length-based group entry insertion. The default implementation assumes that there is
     * only a single entry group.
     *
     * @param entry_groups The entry groups to insert
     * @param inserter_type The type of inserter to use for each group
     */
    virtual void insert_entry_groups(vec<vec<IndexEntry<T>>> &entry_groups, EntryInserterType inserter_type) {
        if (entry_groups.size() != 1) {
            throw std::runtime_error("This index does not support multiple entry groups");
        }
        insert_entries(entry_groups.back(), inserter_type);
    };

    /**
     * @brief Insert entries into the index
     * @param entries The entries to insert
     * @param inserter_type The type of inserter to use
     */
    virtual void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) = 0;

    /**
     * @brief Insert an entry into the index
     * @param entry The entry to insert
     */
    virtual void insert(IndexEntry<T> &entry) = 0;

    /**
     * @brief Construct the index from a dataset
     * @tparam The type of entry to insert into the index
     * @param dataset_path Path to the dataset
     * @param generator Generator to produce the entries from the dataset
     * @param merger Merger to merge Envelope entries if applicable, nullptr otherwise
     * @param inserter_type The type of inserter to use
     * @param num_channels Number of channels in the dataset
     * @param series_len Length of the series
     * @param adapt Whether to adapt the index properties to the dataset
     * @param sample_frac Fraction of the dataset to use for indexing, intended for testing, defaults to 1.0 (index the
     * entire dataset)
     */
    void construct(const str &dataset_path, uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger,
                   EntryInserterType inserter_type, MtsNumChannelsT num_channels, uint series_len, bool adapt = false,
                   Real sample_frac = 1.0);
};

#endif  // INDEX_INDEX_HPP
