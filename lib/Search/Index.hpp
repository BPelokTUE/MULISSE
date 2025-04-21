#ifndef INDEX_HPP
#define INDEX_HPP

#include <fstream>
#include <filesystem>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

#include "Search/SearchMethod.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logging/IndexLogger.hpp"

template <typename T>
struct IndexTraits;

template <>
struct IndexTraits<Paa> {
    using FinalizedTag = PaaTag;
};

template <>
struct IndexTraits<Envelope> {
    using FinalizedTag = EnvelopeTag;
};

/**
 * @brief Interface for finalized indexes
 * @tparam T Traits of the entries in the index
 * */
template <typename T>
    requires ValidEntryTraitsTag<T>
class IFinalizedIndex {
   public:
    virtual ~IFinalizedIndex() = default;

    /**
     * @brief Save the index into a file
     * @param out_file Path to the output file
     * @param ar_type Archive type
     */
    virtual void save(const str &out_file, ArchiveType ar_type) = 0;

    /**
     * @brief Load the index from a file
     * @param in_file Path to the input file
     * @param ar_type Archive type
     */
    virtual void load(const str &in_file, ArchiveType ar_type) = 0;

    /**
     * @brief Get the size on disk in bytes of the saved index
     * @param index_file Path to the index file
     * @param ar_type Archive type
     * @return The size on disk in bytes
     */
    virtual size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type = BINARY) const {
        str index_file_w_ext = add_archive_extension(index_file, ar_type);
        return std::filesystem::exists(index_file_w_ext) ? std::filesystem::file_size(index_file_w_ext) : 0;
    }

    /**
     * @brief Get the length of the series in the index
     * @return The length of the series
     */
    uint get_series_len() const { return m_series_len; }

   protected:
    uint m_series_len;
    MtsNumChannelsT m_num_channels;
};

/**
 * @brief Macro to serializable / deserialize. Not intended to be used directly, but through MAKE_SERIALIZABLE.
 * @param archive_type Archive type, should be an ArchiveType enum value
 * @param stream Stream to serialize / deserialize to / from, should be an std::ofstream or std::ifstream
 * @param operation Operation to perform on the archive, should be a function that takes an archive as an argument
 * @param archive Archive type, should be either "InputArchive" or "OutputArchive"
 */
#define SERIALIZATION_MACRO(archive_type, stream, operation, archive) \
    switch (archive_type) {                                           \
        case JSON: {                                                  \
            cereal::JSON##archive archive(stream);                    \
            operation(archive);                                       \
            break;                                                    \
        }                                                             \
        case BINARY: {                                                \
            cereal::Binary##archive archive(stream);                  \
            operation(archive);                                       \
            break;                                                    \
        }                                                             \
        case NONE: {                                                  \
            break;                                                    \
        }                                                             \
            /* Add new archive types here */                          \
    }

/**
 * @brief Macro to make a class (de)serializable. Intended to be used in classes that inherit from
 * IFinalizedIndex.
 * @param members Members of the class to be serialized
 */
#define MAKE_SERIALIZABLE(members)                                                     \
   private:                                                                            \
    template <typename Archive>                                                        \
    void serialize(Archive &ar) {                                                      \
        ar members;                                                                    \
    }                                                                                  \
    template <typename Archive>                                                        \
    void deserialize(Archive &ar) {                                                    \
        ar members;                                                                    \
    }                                                                                  \
                                                                                       \
   public:                                                                             \
    void save(const str &out_file, ArchiveType ar_type) override {                     \
        std::ofstream ofs(add_archive_extension(out_file, ar_type), std::ios::binary); \
        SERIALIZATION_MACRO(ar_type, ofs, serialize, OutputArchive);                   \
    }                                                                                  \
    void load(const str &in_file, ArchiveType ar_type) override {                      \
        std::ifstream ifs(add_archive_extension(in_file, ar_type), std::ios::binary);  \
        if (!ifs.is_open()) throw std::runtime_error("Could not open index file");     \
        SERIALIZATION_MACRO(ar_type, ifs, deserialize, InputArchive);                  \
    }

// Forward declarations

template <typename T>
    requires DerivedFromEntryData<T>
class IIndex;

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
     * @param inserter_type The type of inserter to use
     * @param num_channels Number of channels in the dataset
     * @param series_len Length of the series
     * @param adapt Whether to adapt the index properties to the dataset
     */
    void construct(const str &dataset_path, uptr<IEntryGenerator<T>> generator, EntryInserterType inserter_type,
                   MtsNumChannelsT num_channels, uint series_len, bool adapt) {
        auto &logger = IndexLogger::get_instance();

        size_t N = get_dataset_size(dataset_path), channel_size = series_len * sizeof(Real),
               series_size = channel_size * num_channels, num_series = N / series_size;

        uint num_length_groups = generator->get_num_len_groups();
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
                OMP_PRAGMA(omp critical) {
                    for (uint l = 0; l < num_length_groups; ++l) {
                        dataset_entry_groups[l].insert(dataset_entry_groups[l].end(), mts_entries[l].begin(),
                                                       mts_entries[l].end());
                    }
                }
            }
        }
        logger.stop_timer(ISC::SUMMARIZATION_TIME_S);

        // TODO: Reconsider if this is a valid approach
        logger.increment_count_col(ISC::NUM_ENTRIES, dataset_entry_groups[0].size());

        if (adapt) adapt_to_dataset_groups(dataset_entry_groups);

        logger.start_timer(ISC::INSERTION_TIME_S);
        insert_entry_groups(dataset_entry_groups, inserter_type);
        logger.stop_timer(ISC::INSERTION_TIME_S);
    }
};

template <typename IndexType>
concept ImplementsIIndex = requires {
    typename IndexType::EntryType;
    requires std::derived_from<IndexType, IIndex<typename IndexType::EntryType>>;
};

/**
 * @brief Interface for entry inserter
 * @tparam IndexType The type of index to insert entries into
 */
template <typename IndexType>
    requires ImplementsIIndex<IndexType>
class IEntryInserter {
    using EntryType = typename IndexType::EntryType;

   public:
    virtual ~IEntryInserter() = default;

    /**
     * @brief Insert entries into the index
     * @param entries The entries to insert
     * @param inserter_type The type of inserter to use
     */
    virtual void insert_entries(vec<IndexEntry<EntryType>> &entries) = 0;
};

/**
 * @brief Abstract class for index-based search methods
 * @tparam FTag The traits of the entries in the index
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether the query is sorted or not
 */
template <typename FTag, SearchType S, DistanceType D, bool QS = false>
    requires ValidEntryTraitsTag<FTag>
class IndexSearchMethod : public ISearchMethod<S, D, QS> {
   protected:
    /**
     * @brief Check if the given entry can be ignored/skipped during search
     * @tparam FTag The type of the index
     * @param query_len Length of the query
     * @param series_len Length of the series
     * @param subs_info Information about the subsequence in the dataset
     * @return `true` if the entry can be skipped, `false` otherwise
     */
    inline bool skip_entry(const uint query_len, const uint series_len, const SubsequenceInfo &subs_info) const {
        if constexpr (std::is_same_v<FTag, PaaTag>) {
            return subs_info.m_length != query_len;
        } else if constexpr (std::is_same_v<FTag, EnvelopeTag>) {
            return series_len - subs_info.m_start_pos < query_len;
        }
        return false;
    }
};

#endif  // INDEX_HPP
