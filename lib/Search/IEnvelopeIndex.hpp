#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include "typedefs.hpp"
#include "Search/SearchOptions.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"

#include <fstream>

class IFinalizedEnvelopeIndex {
   public:
    virtual ~IFinalizedEnvelopeIndex() = default;

    virtual void save(std::ofstream ofs) = 0;

    virtual void load(std::ifstream ifs) = 0;
};

struct EnvelopeEntry {
    vec<Envelope> mts_envelope;
    FilePositionT file_position;

    explicit operator bool() const { return !mts_envelope.empty(); }
};

class IEnvelopeIndex {
   public:
    virtual ~IEnvelopeIndex() = default;

    virtual void insert(const EnvelopeEntry &entry) = 0;

    virtual std::unique_ptr<IFinalizedEnvelopeIndex> finalize() = 0;

    virtual vec<FilePositionT> search(vec<vec<float>> mts, const SearchOptions &search_options) const = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
