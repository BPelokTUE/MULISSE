#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include <fstream>

#include "typedefs.hpp"
#include "Search/IndexOptions.hpp"
#include "Search/SearchOptions.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"

class IFinalizedEnvelopeIndex {
   public:
    virtual ~IFinalizedEnvelopeIndex() = default;

    virtual void save(std::ofstream ofs, ArchiveType ar_type) = 0;

    virtual void load(std::ifstream ifs, ArchiveType ar_type) = 0;
};

struct EnvelopeEntry {
    vec<Envelope> mts_envelope;
    FilePositionT file_position;
};

class IEnvelopeIndex {
   public:
    virtual ~IEnvelopeIndex() = default;

    virtual void insert(const EnvelopeEntry &entry) = 0;

    virtual std::unique_ptr<IFinalizedEnvelopeIndex> finalize() = 0;

    virtual vec<FilePositionT> search(vec<vec<float>> mts, const SearchOptions &search_options) const = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
