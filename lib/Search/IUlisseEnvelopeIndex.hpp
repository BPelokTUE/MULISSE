#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include "typedefs.hpp"
#include "Search/SearchOptions.hpp"
#include "Summarization/iSaxWord.hpp"

#include <fstream>

class IFinalizedUliEnvIndex {
   public:
    virtual void serialize(std::ofstream ofs) = 0;

    virtual void deserialize(std::ifstream ifs) = 0;
};

class IUlisseEnvelopeIndex {
   public:
    virtual ~IUlisseEnvelopeIndex() = default;

    virtual void insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) = 0;

    virtual std::unique_ptr<IFinalizedUliEnvIndex> finalize() = 0;

    virtual vec<FilePositionT> search(vec<vec<float>> mts, const SearchOptions &search_options) const = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
