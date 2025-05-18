#ifndef INDEX_INDEX_HPP
#define INDEX_INDEX_HPP

#include <filesystem>
#include <fstream>

#include "Enums/DistanceType.hpp"
#include "Enums/EntryInserterType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Entry/EntryData.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/EntryMerger/EntryMerger.hpp"
#include "Index/FinalizedIndex.hpp"
#include "Index/Traits/FinalizedTraits.hpp"
#include "Index/Traits/IndexTraits.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

/**
 * @brief Interface for indexes
 * @tparam The type of entry to insert into the index
 * */
template <typename T>
    requires DerivedFromEntryData<T>
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
     */
    void construct(const str &dataset_path, uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger,
                   EntryInserterType inserter_type, MtsNumChannelsT num_channels, uint series_len, bool adapt) {
        auto &logger = IndexLogger::get_instance();

        size_t N = get_dataset_size(dataset_path), channel_size = series_len * sizeof(Real),
               series_size = channel_size * num_channels, num_series = N / series_size;

        uint num_length_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;
        vec<vec<IndexEntry<T>>> dataset_entry_groups(num_length_groups);

        logger.start_timer(ISC::SUMMARIZATION_TIME_S);
        OMP_PRAGMA(omp parallel) {
            std::ifstream data_stream(dataset_path, std::ios::binary);

            OMP_PRAGMA(omp for)
            for (size_t i = 0; i < num_series; ++i) {
                vec<vec<Real>> mts(num_channels, vec<Real>(series_len));
                data_stream.seekg(static_cast<std::streamsize>(i * series_size));
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    data_stream.read(reinterpret_cast<char *>(mts[c].data()),
                                     static_cast<std::streamsize>(channel_size));
                }
                auto mts_entries = generator->get_entries(mts, U(i));
                for (uint l = 0; l < num_length_groups; ++l) {
                    mts_entries[l] = merger->merge_entries(std::move(mts_entries[l]));
                }
                OMP_PRAGMA(omp critical) {
                    for (uint l = 0; l < num_length_groups; ++l) {
                        dataset_entry_groups[l].insert(dataset_entry_groups[l].end(),
                                                       std::make_move_iterator(mts_entries[l].begin()),
                                                       std::make_move_iterator(mts_entries[l].end()));
                    }
                }
            }
        }
        logger.stop_timer(ISC::SUMMARIZATION_TIME_S);

        // TODO: adapt for length groups
        logger.increment_count_col(ISC::NUM_ENTRIES, dataset_entry_groups[0].size());

        if (adapt) adapt_to_dataset_groups(dataset_entry_groups);

        logger.start_timer(ISC::INSERTION_TIME_S);
        insert_entry_groups(dataset_entry_groups, inserter_type);
        logger.stop_timer(ISC::INSERTION_TIME_S);
    }
};

#endif  // INDEX_INDEX_HPP
