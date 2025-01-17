#include "Search/IUlisseEnvelopeIndex.hpp"
#include "iSaxNode.hpp"
#include "iSaxWord.hpp"

class iSaxUlisseEnvelopeIndex : IUlisseEnvelopeIndex {
   public:
    ~iSaxUlisseEnvelopeIndex() = default;
    void insert(const vec<UlisseEnvelope> &envelopes, unsigned long long file_pos) override;
    vec<uint64_t> search(vec<vec<float>> mts, const SearchOptions *search_options) const override;

   private:
    vec<iSaxNode *> m_first_layer_nodes;
    vec<iSaxWord> m_first_layer_words;
};
