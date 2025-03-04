#ifndef ENVELOPE_INDEX_HPP
#define ENVELOPE_INDEX_HPP

#include "Search/Index.hpp"
#include "Util/typedefs.hpp"

class FlatEnvelopeIndex : public IIndex<Envelope>, public IFinalizedIndex<EnvelopeTag> {
   public:
    FlatEnvelopeIndex(uint segment_len, uint pos_per_env);

    FlatEnvelopeIndex() = default;

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override;

    vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &opts,
                             std::ifstream &dataset_ifs) const override;

    const vec<IndexEntry<Envelope>> &get_entries() const;

   private:
    void insert(const IndexEntry<Envelope> &entry) override;

    vec<IndexEntry<Envelope>> m_entries;
    uint m_segment_len, m_pos_per_env;

    MAKE_SERIALIZABLE((m_segment_len, m_pos_per_env, m_entries));
};

#endif  // ENVELOPE_INDEX_HPP
