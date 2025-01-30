#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include <fstream>

#include "typedefs.hpp"
#include "Search/IndexOptions.hpp"
#include "Search/SearchOptions.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"

/** @brief Interface for finalized envelope indexes */
class IFinalizedEnvelopeIndex {
   public:
    virtual ~IFinalizedEnvelopeIndex() = default;

    /**
     * @brief Save the index into a file
     *
     * @param ofs Output file stream
     * @param ar_type Archive type
     */
    virtual void save(std::ofstream ofs, ArchiveType ar_type) = 0;

    /**
     * @brief Load the index from a file
     *
     * @param ifs Input file stream
     * @param ar_type Archive type
     */
    virtual void load(std::ifstream ifs, ArchiveType ar_type) = 0;

    /**
     * @brief Search for multivariate subsequence using the index
     *
     * @param mts Multivariate subsequence to search for
     * @param search_options Search options
     * @return The start positions of the subsequences in the result set
     */
    virtual vec<FilePositionT> search(const vec<vec<float>> &mts, const SearchOptions &search_options) const = 0;
};

/** @brief Entry to insert into the envelope index */
struct EnvelopeEntry {
    /** @brief Multivariate time series envelope */
    vec<Envelope> mts_envelope;
    /** @brief Starting position of the first channel of the time series in the file */
    FilePositionT file_position;
};

/** @brief Interface for envelope indexes */
class IEnvelopeIndex {
   public:
    virtual ~IEnvelopeIndex() = default;

    /**
     * @brief Insert an envelope entry into the index
     *
     * @param entry The envelope entry to insert
     */
    virtual void insert(const EnvelopeEntry &entry) = 0;

    /**
     * @brief Finalize the index
     *
     * Creates a finalized index, that can no longer be inserted into, but can be used for searching.
     *
     * @return A unique pointer to the finalized envelope index
     */
    virtual std::unique_ptr<IFinalizedEnvelopeIndex> finalize() = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
