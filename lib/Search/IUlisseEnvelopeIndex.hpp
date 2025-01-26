#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include "typedefs.hpp"
#include "Search/SearchOptions.hpp"
#include "Summarization/iSaxWord.hpp"

#include <fstream>

class IFinalizedUliEnvIndex {
   public:
    virtual ~IFinalizedUliEnvIndex() = default;

    virtual void serialize(std::ofstream ofs) = 0;

    virtual void deserialize(std::ifstream ifs) = 0;
};

struct EnvelopeEntry {
    vec<UlisseEnvelope> mts_envelope;
    FilePositionT file_position;

    explicit operator bool() const { return !mts_envelope.empty(); }
};

class IUlisseEnvelopeIndex {
   public:
    virtual ~IUlisseEnvelopeIndex() = default;

    virtual void insert(const EnvelopeEntry &entry) = 0;

    virtual std::unique_ptr<IFinalizedUliEnvIndex> finalize() = 0;

    virtual vec<FilePositionT> search(vec<vec<float>> mts, const SearchOptions &search_options) const = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
