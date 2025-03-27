#ifndef INDEX_HPP
#define INDEX_HPP

#include <fstream>

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
#include "Util/Logger.hpp"

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

/** @brief Interface for finalized indexes */
template <typename T>
    requires ValidSaxTraitsTag<T>
class IFinalizedIndex : public ISearchMethod {
   public:
    virtual ~IFinalizedIndex() = default;

    /**
     * @brief Save the index into a file
     * @param ofs Output file stream
     * @param ar_type Archive type
     */
    virtual void save(std::ofstream &ofs, ArchiveType ar_type) = 0;

    /**
     * @brief Load the index from a file
     * @param ifs Input file stream
     * @param ar_type Archive type
     */
    virtual void load(std::ifstream &ifs, ArchiveType ar_type) = 0;

    /**
     * @brief Get the length of the series in the index
     * @return The length of the series
     */
    uint get_series_len() const { return m_series_len; }

   protected:
    uint m_series_len, m_pos_per_env;
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
#define MAKE_SERIALIZABLE(members)                                    \
   private:                                                           \
    template <typename Archive>                                       \
    void serialize(Archive &ar) {                                     \
        ar members;                                                   \
    }                                                                 \
    template <typename Archive>                                       \
    void deserialize(Archive &ar) {                                   \
        ar members;                                                   \
    }                                                                 \
                                                                      \
   public:                                                            \
    void save(std::ofstream &ofs, ArchiveType ar_type) override {     \
        SERIALIZATION_MACRO(ar_type, ofs, serialize, OutputArchive);  \
    }                                                                 \
    void load(std::ifstream &ifs, ArchiveType ar_type) override {     \
        SERIALIZATION_MACRO(ar_type, ifs, deserialize, InputArchive); \
    }

// Forward declarations

template <typename T>
    requires DerivedFromEntryData<T>
class IIndex;

template <typename IndexType>
concept ImplementsIIndex = requires {
    typename IndexType::EntryType;
    requires std::derived_from<IndexType, IIndex<typename IndexType::EntryType>>;
};

template <typename IndexType>
    requires ImplementsIIndex<IndexType>
class IEntryInserter;

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
     * @brief Construct the index from a dataset
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

        vec<IndexEntry<T>> dataset_entries;

        logger.start_timer(ISC::SUMMARIZATION_TIME_S);
        OMP_PRAGMA(omp parallel) {
            std::ifstream data_stream(dataset_path, std::ios::binary);
            OMP_PRAGMA(omp for)
            for (size_t i = 0; i < num_series; ++i) {
                vec<vec<Real>> mts(num_channels, vec<Real>(series_len));
                data_stream.seekg(i * series_size);
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    data_stream.read(reinterpret_cast<char *>(mts[c].data()), channel_size);
                }
                auto mts_entries = generator->get_entries(mts, i);
                OMP_PRAGMA(omp critical) {
                    dataset_entries.insert(dataset_entries.end(), mts_entries.begin(), mts_entries.end());
                }
            }
        }
        logger.stop_timer(ISC::SUMMARIZATION_TIME_S);

        if (adapt) adapt_to_dataset(dataset_entries);

        logger.increment_count_col(ISC::NUM_ENTRIES, dataset_entries.size());
        logger.start_timer(ISC::INSERTION_TIME_S);
        insert_entries(dataset_entries, inserter_type);
        logger.stop_timer(ISC::INSERTION_TIME_S);
    }

    /**
     * @brief Adapt the index to the dataset entries
     * @param dataset_entries The entries to adapt to
     */
    virtual void adapt_to_dataset(const vec<IndexEntry<T>> &dataset_entries) {}

    /**
     * @brief Finalize the index
     *
     * Creates a finalized index, that can no longer be inserted into, but can be used for searching.
     *
     * @return A unique pointer to the finalized index
     */
    virtual uptr<IFinalizedIndex<FTag>> finalize() = 0;

    /**
     * @brief Insert entries into the index
     * @param entries The entries to insert
     * @param inserter_type The type of inserter to use
     */
    virtual void insert_entries(const vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) = 0;

    /**
     * @brief Insert an entry into the index
     * @param entry The entry to insert
     */
    virtual void insert(const IndexEntry<T> &entry) = 0;
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
    virtual void insert_entries(const vec<IndexEntry<EntryType>> &entries) = 0;
};

#endif  // INDEX_HPP
