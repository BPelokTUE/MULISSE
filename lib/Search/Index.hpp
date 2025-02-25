#ifndef INDEX_HPP
#define INDEX_HPP

#include <fstream>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

#include "Util/typedefs.hpp"
#include "Search/SearchMethod.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/Paa.hpp"

template <typename T>
struct iSaxIndexTraits;

template <>
struct iSaxIndexTraits<Paa> {
    using FinalizedTag = PaaTag;
};

template <>
struct iSaxIndexTraits<Envelope> {
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
     *
     * @param ofs Output file stream
     * @param ar_type Archive type
     */
    virtual void save(std::ofstream &ofs, ArchiveType ar_type) = 0;

    /**
     * @brief Load the index from a file
     *
     * @param ifs Input file stream
     * @param ar_type Archive type
     */
    virtual void load(std::ifstream &ifs, ArchiveType ar_type) = 0;

    /**
     * @brief Get the length of the series in the index
     *
     * @return The length of the series
     */
    uint get_series_len() const { return m_series_len; }

   protected:
    uint m_series_len, m_pos_per_env;
    MtsNumChannelsT m_num_channels;
};

/**
 * @brief Macro to serializable / deserialize. Not intended to be used directly, but through MAKE_SERIALIZABLE.
 *
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
 *
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

/**
 * @brief Interface for indexes
 * @tparam The type of entry to insert into the index
 * */
template <typename T>
    requires DerivedFromEntryData<T>
class IIndex {
    using FTag = typename iSaxIndexTraits<T>::FinalizedTag;

   public:
    virtual ~IIndex() = default;

    void construct(const str &dataset_path, IEntryGenerator<T> *generator, MtsNumChannelsT num_channels,
                   uint series_len) {
        uint N = get_dataset_size(dataset_path), channel_size = series_len * sizeof(float),
             series_size = channel_size * num_channels;
        uint num_series = N / series_size;

        vec<IndexEntry<T>> dataset_entries;

#ifndef DISABLE_PARALLELISM
#pragma omp parallel
#endif
        {
            std::ifstream data_stream(dataset_path, std::ios::binary);
#ifndef DISABLE_PARALLELISM
#pragma omp for
#endif
            for (size_t i = 0; i < num_series; ++i) {
                vec<vec<float>> mts(num_channels, vec<float>(series_len));
                data_stream.seekg(i * series_size);
                for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                    data_stream.read(reinterpret_cast<char *>(mts[c].data()), channel_size);
                }
                auto mts_entries = generator->get_entries(mts, i);
#ifndef DISABLE_PARALLELISM
#pragma omp critical
#endif
                {
                    dataset_entries.insert(dataset_entries.end(), mts_entries.begin(), mts_entries.end());
                }
            }
        }

        // TODO: adapt stuff based on entries, e.g. change breakpoint distribution mean

        for (auto &entry : dataset_entries) insert(std::move(entry));
    }

    /**
     * @brief Finalize the index
     *
     * Creates a finalized index, that can no longer be inserted into, but can be used for searching.
     *
     * @return A unique pointer to the finalized index
     */
    virtual uptr<IFinalizedIndex<FTag>> finalize() = 0;

   private:
    /**
     * @brief Insert an entry into the index
     *
     * @param entry The entry to insert
     */
    virtual void insert(const IndexEntry<T> &entry) = 0;
};

#endif  // INDEX_HPP
