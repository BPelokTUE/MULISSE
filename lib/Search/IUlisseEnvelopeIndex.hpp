#ifndef I_ULISSE_ENVELOPE_INDEX_HPP
#define I_ULISSE_ENVELOPE_INDEX_HPP

#include "typedefs.hpp"
#include "SearchOptions.hpp"

class IUlisseEnvelopeIndex {
   public:
    virtual ~IUlisseEnvelopeIndex() = default;
    virtual void insert(const vec<UlisseEnvelope> &envelopes, unsigned long long file_pos) = 0;
    virtual vec<uint64_t> search(vec<vec<float>> mts, const SearchOptions *search_options) const = 0;
};

#endif  // I_ULISSE_ENVELOPE_INDEX_HPP
