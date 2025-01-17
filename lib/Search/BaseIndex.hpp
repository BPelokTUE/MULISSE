#include "typedefs.hpp"
#include "SearchOptions.hpp"

class IUlisseEnvelopeIndex {
   public:
    virtual ~IUlisseEnvelopeIndex() = default;
    virtual void insert(const UlisseEnvelope &envelope, unsigned long long file_pos) = 0;
    virtual vec<uint64_t> search(vec<vec<float>> mts, const SearchOptions *search_options) const = 0;
};
