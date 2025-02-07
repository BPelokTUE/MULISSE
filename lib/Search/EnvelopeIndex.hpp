#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include <fstream>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

#include "Util/typedefs.hpp"
#include "Search/IndexOptions.hpp"
#include "Search/SearchOptions.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"

/** @brief Interface for finalized envelope indexes */
class IEnvelopeFinalizedIndex {
   public:
    virtual ~IEnvelopeFinalizedIndex() = default;

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
     * @brief Search for multivariate subsequence using the index
     *
     * @param query Multivariate subsequence to search for
     * @param search_options Search options
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &search_options) const = 0;

    /**
     * @brief Get the length of the series in the index
     *
     * @return The length of the series
     */
    uint get_series_len() const;

    /**
     * @brief Get the number of positions per envelope in the index
     *
     * @return The number of positions per envelope
     */
    uint get_pos_per_env() const;

    /**
     * @brief Get the number of channels in the index
     *
     * @return The number of channels
     */
    MtsNumChannelsT get_num_channels() const;

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
            /* Add new archive types here */                          \
    }

/**
 * @brief Macro to make a class (de)serializable. Intended to be used in classes that inherit from
 * IEnvelopeFinalizedIndex.
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

/** @brief Interface for envelope indexes */
class IEnvelopeIndex {
   public:
    virtual ~IEnvelopeIndex() = default;

    void construct(const str &dataset_path, IEnvelopeGenerator *generator, MtsNumChannelsT num_channels,
                   uint series_len);

    /**
     * @brief Finalize the index
     *
     * Creates a finalized index, that can no longer be inserted into, but can be used for searching.
     *
     * @return A unique pointer to the finalized envelope index
     */
    virtual std::unique_ptr<IEnvelopeFinalizedIndex> finalize() = 0;

   private:
    /**
     * @brief Insert an envelope entry into the index
     *
     * @param entry The envelope entry to insert
     */
    virtual void insert(const EnvelopeEntry &entry) = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
